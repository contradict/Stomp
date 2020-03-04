#include <stdbool.h>
#include <machine/endian.h>
#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_usart.h"
#include "modbus_uart.h"
#include "cmsis_os.h"
#include "modbus.h"
#include "status_led.h"

#define MAXPACKET 128

#define CRC_POLYNOMIAL_MODBUS 0x8005

#define HIGH_BYTE(x) ((x>>8) & 0xff)
#define LOW_BYTE(x) (x&0xff)

#define WORD(data, offset) (__builtin_bswap16(*((uint16_t *)(data + offset))))

enum ModbusSignalEvent
{
    SIGNAL_TXCPLT = 1,
    SIGNAL_RXCPLT = 2,
    SIGNAL_RXTO   = 4,
    SIGNAL_ERROR  = 8
};

enum ModbusFunctionCode
{
    READ_COILS                    = 1,
    READ_DISCRETE_INPUTS          = 2,
    READ_HOLDING_REGISTERS        = 3,
    READ_INPUT_REGISTERS          = 4,
    WRITE_SINGLE_COIL             = 5,
    WRITE_SINGLE_REGISTER         = 6,
    READ_EXCEPTION_STATUS         = 7,
    DIAGNOSTIC                    = 8,
    GET_COM_EVENT_COUNTER         = 11,
    GET_COM_EVENT_LOG             = 12,
    WRITE_MULTIPLE_COILS          = 15,
    WRITE_MULTIPLE_REGISTERS      = 16 ,
    REPORT_SERVER_ID              = 17,
    READ_FILE_RECORD              = 20,
    WRITE_FILE_RECORD             = 21,
    READ_WRITE_MULTIPLE_REGISTERS = 22,
    READ_FIFO_QUEUE               = 24
};

struct MODBUS_parameters
{
    uint8_t address;
};

static void MODBUS_Thread(const void* args);

osThreadDef(modbus, MODBUS_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
static osThreadId modbus;

static struct MODBUS_parameters modbus_parameters __attribute__ ((section (".storage.modbus"))) = {
    .address = 0x55
};

static CRC_HandleTypeDef hcrc;

extern struct MODBUS_Coil modbus_coils[];
extern struct MODBUS_DiscreteInput modbus_discrete_inputs[];
extern struct MODBUS_InputRegister modbus_input_registers[];
extern struct MODBUS_HoldingRegister modbus_holding_registers[];

void MODBUS_Init()
{
    MODBUS_UART_Init();
    hcrc.Instance = CRC;
    hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    hcrc.Init.GeneratingPolynomial = CRC_POLYNOMIAL_MODBUS;
    hcrc.Init.CRCLength = CRC_POLYLENGTH_16B;
    hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    hcrc.Init.InitValue = 0xFFFF;
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
	hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
	hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    HAL_CRC_Init(&hcrc);

    modbus = osThreadCreate(osThread(modbus), NULL);
}

uint16_t MODBUS_crc(uint8_t* buffer, uint16_t count)
{
    return HAL_CRC_Calculate(&hcrc, (uint32_t *)buffer, count);
}

/*
 * MODBUS_FindCoil
 * MODBUS_FindDiscreteInput
 * MODBUS_FindInputRegister
 * MODBUS_FindHoldingRegister
 */
#define MODBUS_FIND(findtype) \
    struct MODBUS_##findtype *MODBUS_Find##findtype(struct MODBUS_##findtype *obj, uint16_t address) \
    { \
        for(size_t i=0; obj[i].address; i++) \
        { \
            if(obj[i].address == address) \
            { \
                return obj + i; \
            } \
        } \
        return (struct MODBUS_##findtype*)0; \
    }
MODBUS_FIND(Coil)
MODBUS_FIND(DiscreteInput)
MODBUS_FIND(InputRegister)
MODBUS_FIND(HoldingRegister)

enum ModbusExceptionCode {
// 01 ILLEGAL FUNCTION CODE The function code received in the query is not an
// allowable action for the slave.  If a Poll Program Complete command was
// issued, this code indicates that no program function preceded it.
    ILLEGAL_FUNCTION_CODE = 1,
// 02 ILLEGAL DATA ADDRESS The data address received in the query is not an
// allowable address for the slave.
    ILLEGAL_DATA_ADDRESS  = 2,
// 03 ILLEGAL DATA VALUE A value contained in the query data field is not an
// allowable value for the slave.
    ILLEGAL_DATA_VALUE    = 3,
// 04 SLAVE DEVICE FAILURE An unrecoverable error occurred while the slave was
// attempting to perform the requested action.
    SLAVE_DEVICE_FAILURE  = 4,
// 05 ACKNOWLEDGE The slave has accepted the request and is processing it, but a
// long duration of time will be required to do so. This response is returned to
// prevent a timeout error from occurring in the master. The master can next
// issue a Poll Program Complete message to determine if processing is
// completed.
    ACKNOWLEDGE           = 5,
// 06 SLAVE DEVICE BUSY The slave is engaged in processing a long–duration
// program command. The master should retransmit the message later when the
// slave is free
    SLAVE_DEVICE_BUSY     = 6,
};

static size_t MODBUS_ExceptionResponse(enum ModbusFunctionCode function, enum ModbusExceptionCode code, uint8_t *txBuffer, size_t txLength)
{
    LED_BlinkOne(2, 0, 127, 50);
    if(txLength>5) // space for CRC
    {
        txBuffer[0] = modbus_parameters.address;
        txBuffer[1] = 0x80 | function;
        txBuffer[2] = code;
        return 3;
    }
    else
    {
        return 0;
    }
}

static size_t MODBUS_ReadCoils(uint16_t start, uint16_t count, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_Coil *coil;
    size_t responseLength = 3;

    txBuffer[0] = modbus_parameters.address;
    txBuffer[1] = READ_COILS;
    uint8_t *byte_count = &(txBuffer[2]);
    *byte_count = 0;
    uint8_t *data = &(txBuffer[3]);
    *data = 0;

    for(size_t i=start;i<start + count;i++)
    {
        coil = MODBUS_FindCoil(modbus_coils, i);
        if(coil)
        {
            size_t bitnum = ((i-start) % 8);
            if(bitnum == 0)
            {
                *byte_count += 1;
            }
            bool c = coil->read(coil->context);
            if(c)
            {
                *data |= (0x01 << bitnum);
            }
            if(bitnum == 7)
            {
                data += 1;
            }
        }
        else
        {
            return MODBUS_ExceptionResponse(READ_COILS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    return responseLength + *byte_count;
}

static size_t MODBUS_ReadDiscreteInputs(uint16_t start, uint16_t count, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_DiscreteInput *input;
    size_t responseLength = 3;

    txBuffer[0] = modbus_parameters.address;
    txBuffer[1] = READ_DISCRETE_INPUTS;
    uint8_t *byte_count = &(txBuffer[2]);
    *byte_count = 0;
    uint8_t *data = &(txBuffer[3]);
    *data = 0;

    for(size_t i=start;i<start + count;i++)
    {
        input = MODBUS_FindDiscreteInput(modbus_discrete_inputs, i);
        if(input)
        {
            size_t bitnum = ((i-start) % 8);
            if(bitnum == 0)
            {
                *byte_count += 1;
            }
            bool c = input->read(input->context);
            if(c)
            {
                *data |= (0x01 << bitnum);
            }
            if(bitnum == 7)
            {
                data += 1;
                *data = 0;
            }
        }
        else
        {
            return MODBUS_ExceptionResponse(READ_DISCRETE_INPUTS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    return responseLength + *byte_count;
}

static size_t MODBUS_ReadHoldingRegister(uint16_t start, uint16_t count, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_HoldingRegister *hold;
    size_t responseLength = 3;

    txBuffer[0] = modbus_parameters.address;
    txBuffer[1] = READ_HOLDING_REGISTERS;
    uint8_t *byte_count = &(txBuffer[2]);
    *byte_count = 0;
    uint16_t *data = (uint16_t *)&(txBuffer[3]);
    *data = 0;

    for(size_t i=start;i<start + count;i++)
    {
        hold = MODBUS_FindHoldingRegister(modbus_holding_registers, i);
        if(hold)
        {
            *byte_count += 2;
            *data = __htons(hold->read(hold->context));
        }
        else
        {
            return MODBUS_ExceptionResponse(READ_HOLDING_REGISTERS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    return responseLength + *byte_count;
}

static size_t MODBUS_ReadInputRegister(uint16_t start, uint16_t count, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_InputRegister *input;
    size_t responseLength = 3;

    txBuffer[0] = modbus_parameters.address;
    txBuffer[1] = READ_INPUT_REGISTERS;
    uint8_t *byte_count = &(txBuffer[2]);
    *byte_count = 0;
    uint16_t *data = (uint16_t *)&(txBuffer[3]);
    *data = 0;

    for(size_t i=start;i<start + count;i++)
    {
        input = MODBUS_FindInputRegister(modbus_input_registers, i);
        if(input)
        {
            *byte_count += 2;
            *data = __htons(input->read(input->context));
        }
        else
        {
            return MODBUS_ExceptionResponse(READ_INPUT_REGISTERS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    return responseLength + *byte_count;
}

size_t MODBUS_Process(uint8_t *pdu, size_t pdu_length, uint8_t *txBuffer, size_t txLength)
{
    size_t responseLength = 0;
    uint16_t start, count;

    switch(pdu[0])
    {
        case READ_COILS:
            if(pdu_length != 5)
            {
                // We can't figure out what the address should have been so it
                // must be wrong.
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                start = WORD(pdu, 1);
                count = WORD(pdu, 3);
                responseLength = MODBUS_ReadCoils(start, count, txBuffer, txLength);
            }
            break;
        case READ_DISCRETE_INPUTS:
            if(pdu_length != 5)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                start = WORD(pdu, 1);
                count = WORD(pdu, 3);
                responseLength = MODBUS_ReadDiscreteInputs(start, count, txBuffer, txLength);
            }
            break;
        case READ_HOLDING_REGISTERS:
            if(pdu_length != 5)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                start = WORD(pdu, 1);
                count = WORD(pdu, 3);
                responseLength = MODBUS_ReadHoldingRegister(start, count, txBuffer, txLength);
            }
            break;
        case READ_INPUT_REGISTERS:
            if(pdu_length != 5)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                start = WORD(pdu, 1);
                count = WORD(pdu, 3);
                responseLength = MODBUS_ReadInputRegister(start, count, txBuffer, txLength);
            }
            break;
        case WRITE_SINGLE_COIL:
            break;
        case WRITE_SINGLE_REGISTER:
            break;
        case WRITE_MULTIPLE_COILS:
            break;
        case WRITE_MULTIPLE_REGISTERS:
            break;
        default:
            responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_FUNCTION_CODE, txBuffer, txLength);
            break;
    }
    if(responseLength > 0 && (responseLength + 2) < txLength)
    {
        uint16_t crc = MODBUS_crc(txBuffer, responseLength);
        txBuffer[responseLength++] = LOW_BYTE(crc);
        txBuffer[responseLength++] = HIGH_BYTE(crc);
    }
    return responseLength;
}

void MODBUS_Thread(const void *args)
{
    (void)args;
    uint8_t rxBuffer[MAXPACKET];
    uint8_t txBuffer[MAXPACKET];
    osEvent e;
    size_t bytes_received;
    size_t responseLength;
    enum ModbusSignalEvent evt;

    // start listening
    HAL_UART_Receive_IT(&modbus_uart, rxBuffer, MAXPACKET);
    LL_USART_EnableIT_RTO(modbus_uart.Instance);
    while(1)
    {
        // waitfor something to happen
        e = osSignalWait(SIGNAL_RXTO | SIGNAL_ERROR | SIGNAL_RXCPLT | SIGNAL_TXCPLT, osWaitForever);
        if(e.status != osEventSignal)
        {
            continue;
        }
        evt = e.value.signals;

        // Handle event
        switch((enum ModbusSignalEvent)e.value.v)
        {
            case SIGNAL_RXTO:
                bytes_received = modbus_uart.pRxBuffPtr - rxBuffer;
                if(rxBuffer[0] == modbus_parameters.address &&
                        bytes_received > 4 &&
                        MODBUS_crc(rxBuffer, bytes_received) == 0)
                {
                    LED_BlinkOne(2, 2, 127, 50);
                    responseLength = MODBUS_Process(rxBuffer + 1, bytes_received - 3, txBuffer, MAXPACKET);
                }
                break;

            case SIGNAL_RXCPLT:
                // Filled buffer with no timeout
                break;

            case SIGNAL_TXCPLT:
                break;

            case SIGNAL_ERROR:
                break;
        }
		if(responseLength > 0)
        {
            HAL_UART_Transmit_IT(&modbus_uart, txBuffer, responseLength);
            responseLength = 0;
        }
        else
        {
            HAL_UART_Receive_IT(&modbus_uart, rxBuffer, MAXPACKET);
            LL_USART_EnableIT_RTO(modbus_uart.Instance);
        }

    }
}

void MODBUS_TxCplt(UART_HandleTypeDef *huart)
{
    (void)huart;
    osSignalSet(modbus, SIGNAL_TXCPLT);
}

void MODBUS_RxCplt(UART_HandleTypeDef *huart)
{
    (void)huart;
    osSignalSet(modbus, SIGNAL_RXCPLT);
}

void MODBUS_RxTo(UART_HandleTypeDef *huart)
{
    (void)huart;
    osSignalSet(modbus, SIGNAL_RXTO);
}

void MODBUS_UARTError(UART_HandleTypeDef *huart)
{
    (void)huart;
    osSignalSet(modbus, SIGNAL_ERROR);
}
