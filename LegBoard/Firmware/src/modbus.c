#include <stdbool.h>
#include <string.h>
#include <machine/endian.h>
#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_usart.h"
#include "modbus_uart.h"
#include "cmsis_os.h"
#include "modbus.h"
#include "status_led.h"
#include "enfield.h"

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

osThreadDef(modbus, MODBUS_Thread, osPriorityNormal, 1, configMINIMAL_STACK_SIZE);
static osThreadId modbus;

static struct ModbusThreadState {
    uint8_t rxBuffer[MAXPACKET];
    uint8_t txBuffer[MAXPACKET];
    osEvent e;
    size_t bytes_received;
    size_t responseLength;
    enum ModbusSignalEvent evt;
} modbus_state;

osMailQDef(enfieldQ, 32, struct EnfieldResponse);
static osMailQId enfieldQ;

static struct MODBUS_parameters modbus_parameters __attribute__ ((section (".storage.modbus"))) = {
    .address = 0x55
};

static CRC_HandleTypeDef hcrc;

extern struct MODBUS_Coil modbus_coils[];
extern struct MODBUS_DiscreteInput modbus_discrete_inputs[];
extern struct MODBUS_InputRegister modbus_input_registers[];
extern struct MODBUS_HoldingRegister modbus_holding_registers[];

osMutexDef(crcmutex);
static osMutexId crcmutex;

UART_HandleTypeDef modbus_uart;


void MODBUS_Init()
{
    crcmutex = osMutexCreate(osMutex(crcmutex));

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

    bzero(&modbus_state, sizeof(modbus_state));

    modbus = osThreadCreate(osThread(modbus), &modbus_state);
    enfieldQ = osMailCreate(osMailQ(enfieldQ), modbus);
}

void MODBUS_UART_Init()
{
    modbus_uart.Instance = MODBUS_UART_Instance;
    modbus_uart.Init.BaudRate = 1000000;
    modbus_uart.Init.WordLength = UART_WORDLENGTH_8B;
    modbus_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    modbus_uart.Init.Mode = UART_MODE_TX_RX;
    modbus_uart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    modbus_uart.Init.OverSampling = UART_OVERSAMPLING_8;
    modbus_uart.Init.Parity = UART_PARITY_NONE;
    modbus_uart.Init.StopBits = UART_STOPBITS_1;
    HAL_RS485Ex_Init(&modbus_uart, UART_DE_POLARITY_HIGH, 8, 8);
    LL_USART_SetRxTimeout(MODBUS_UART_Instance, 20);
    LL_USART_EnableRxTimeout(MODBUS_UART_Instance);
}

uint16_t MODBUS_crc(uint8_t* buffer, uint16_t count)
{
    uint16_t crc;
    osMutexWait(crcmutex, osWaitForever);
    crc = HAL_CRC_Calculate(&hcrc, (uint32_t *)buffer, count);
    osMutexRelease(crcmutex);
    return crc;
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
    int err;

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
            bool c;
            err = coil->read(coil->context, &c);
            if(0 != err)
            {
                return MODBUS_ExceptionResponse(READ_COILS, err, txBuffer, txLength);
            }
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
    int err;

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
            bool c;
            err = input->read(input->context, &c);
            if( 0 != err)
            {
                return MODBUS_ExceptionResponse(READ_DISCRETE_INPUTS, err, txBuffer, txLength);
            }
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
    int err;

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
            err = hold->read(hold->context, data);
            if( 0 != err)
            {
                return MODBUS_ExceptionResponse(READ_HOLDING_REGISTERS, err, txBuffer, txLength);
            }
            *data = __htons(*data);
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
    int err;

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
            err = input->read(input->context, data);
            if(0 != err)
            {
                return MODBUS_ExceptionResponse(READ_INPUT_REGISTERS, err, txBuffer, txLength);
            }
            *data = __htons(*data);
        }
        else
        {
            return MODBUS_ExceptionResponse(READ_INPUT_REGISTERS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    return responseLength + *byte_count;
}

static size_t MODBUS_WriteSingleCoil(uint16_t address, uint16_t value, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_Coil *coil = MODBUS_FindCoil(modbus_coils, address);
    int err;
    if(coil)
    {
        txBuffer[0] = modbus_parameters.address;
        txBuffer[1] = WRITE_SINGLE_COIL;
        *((uint16_t *)&txBuffer[2]) = __builtin_bswap16(address);
        *((uint16_t *)&txBuffer[4]) = __builtin_bswap16(value);
        err = coil->write(coil->context, value);
        if( 0 != err)
        {
            return MODBUS_ExceptionResponse(WRITE_SINGLE_COIL, err, txBuffer, txLength);
        }
        else
        {
            return 6; 
        }
    }
    else
    {
        return MODBUS_ExceptionResponse(WRITE_SINGLE_COIL, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
    }
}

static size_t MODBUS_WriteSingleRegister(uint16_t address, uint16_t value, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_HoldingRegister *hold = MODBUS_FindHoldingRegister(modbus_holding_registers, address);
    int err;
    if(hold)
    {
        txBuffer[0] = modbus_parameters.address;
        txBuffer[1] = WRITE_SINGLE_REGISTER;
        *((uint16_t *)&txBuffer[2]) = __builtin_bswap16(address);
        *((uint16_t *)&txBuffer[4]) = __builtin_bswap16(value);
        err = hold->write(hold->context, value);
        if(0 != err)
        {
            return MODBUS_ExceptionResponse(WRITE_SINGLE_REGISTER, err, txBuffer, txLength);
        }
        else
        {
            return 6;
        }
    }
    else
    {
        return MODBUS_ExceptionResponse(WRITE_SINGLE_REGISTER, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
    }
}

static size_t MODBUS_WriteMultipleCoils(uint16_t start, uint16_t count, uint8_t *data, uint8_t byte_count, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_Coil *coil;
    size_t responseLength = 6;
    int err;

    txBuffer[0] = modbus_parameters.address;
    txBuffer[1] = WRITE_MULTIPLE_COILS;
    *((uint16_t *)&txBuffer[2]) = __builtin_bswap16(start);
    uint16_t *wcount = ((uint16_t *)&txBuffer[4]);
    *wcount = 0;

    for(size_t i=start, bit_index=0, data_index=0;
        i<start + count && data_index < byte_count;
        i++, bit_index++)
    {
        if(bit_index == 8)
        {
            data_index += 1;
            bit_index = 0;
        }
        coil = MODBUS_FindCoil(modbus_coils, i);
        if(coil)
        {
            *wcount += 1;
            err = coil->write(coil->context, data[data_index] & (1<<bit_index));
            if(0 != err)
            {
                return MODBUS_ExceptionResponse(WRITE_MULTIPLE_COILS, err, txBuffer, txLength);
            }
        }
        else
        {
            return MODBUS_ExceptionResponse(WRITE_MULTIPLE_COILS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    *wcount = __builtin_bswap16(*wcount);
    return responseLength;
}

static size_t MODBUS_WriteMultipleRegisters(uint16_t start, uint16_t count, uint16_t *data, uint8_t byte_count, uint8_t *txBuffer, size_t txLength)
{
    struct MODBUS_HoldingRegister *hold;
    size_t responseLength = 6;
    int err;

    txBuffer[0] = modbus_parameters.address;
    txBuffer[1] = WRITE_MULTIPLE_REGISTERS;
    *((uint16_t *)&txBuffer[2]) = __builtin_bswap16(start);
    uint16_t *wcount = ((uint16_t *)&txBuffer[4]);
    *wcount = 0;

    for(size_t i=start, data_index=0;
        i<start + count && 2*data_index < byte_count;
        i++, data_index++)
    {
        hold = MODBUS_FindHoldingRegister(modbus_holding_registers, i);
        if(hold)
        {
            *wcount += 1;
            err = hold->write(hold->context, __builtin_bswap16(data[data_index]));
            if(0 != err)
            {
                return MODBUS_ExceptionResponse(WRITE_MULTIPLE_COILS, err, txBuffer, txLength);
            }
        }
        else
        {
            return MODBUS_ExceptionResponse(WRITE_MULTIPLE_COILS, ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
        }
    }
    *wcount = __builtin_bswap16(*wcount);
    return responseLength;
}

size_t MODBUS_Process(uint8_t *pdu, size_t pdu_length, uint8_t *txBuffer, size_t txLength)
{
    size_t responseLength = 0;
    uint16_t start, count, address, value;
    uint8_t byte_count;

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
            if(pdu_length != 5)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                address = WORD(pdu, 1);
                value = WORD(pdu, 3);
                responseLength = MODBUS_WriteSingleCoil(address, value, txBuffer, txLength);
            }
            break;
        case WRITE_SINGLE_REGISTER:
            if(pdu_length != 5)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                address = WORD(pdu, 1);
                value = WORD(pdu, 3);
                responseLength = MODBUS_WriteSingleRegister(address, value, txBuffer, txLength);
            }
            break;
        case WRITE_MULTIPLE_COILS:
            if(pdu_length < 6)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                start = WORD(pdu, 1);
                count = WORD(pdu, 3);
                byte_count = pdu[5];
                responseLength = MODBUS_WriteMultipleCoils(start, count, pdu+6, byte_count, txBuffer, txLength);
            }
            break;
        case WRITE_MULTIPLE_REGISTERS:
            if(pdu_length < 6)
            {
                responseLength = MODBUS_ExceptionResponse(pdu[0], ILLEGAL_DATA_ADDRESS, txBuffer, txLength);
            }
            else
            {
                start = WORD(pdu, 1);
                count = WORD(pdu, 3);
                byte_count = pdu[5];
                responseLength = MODBUS_WriteMultipleRegisters(start, count, (uint16_t *)(pdu+6), byte_count, txBuffer, txLength);
            }
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
    struct ModbusThreadState *st = (struct ModbusThreadState *)args;
    // start listening
    HAL_UART_Receive_IT(&modbus_uart, st->rxBuffer, MAXPACKET);
    LL_USART_EnableIT_RTO(modbus_uart.Instance);
    while(1)
    {
        // waitfor something to happen
        st->e = osSignalWait(SIGNAL_RXTO | SIGNAL_ERROR | SIGNAL_RXCPLT | SIGNAL_TXCPLT, osWaitForever);
        if(st->e.status != osEventSignal)
        {
            continue;
        }
        st->evt = st->e.value.signals;

        // Handle event
        if(st->evt & SIGNAL_RXTO)
        {
            st->bytes_received = modbus_uart.pRxBuffPtr - st->rxBuffer;
            if(st->rxBuffer[0] == modbus_parameters.address &&
                    st->bytes_received > 4 &&
                    MODBUS_crc(st->rxBuffer, st->bytes_received) == 0)
            {
                st->responseLength = MODBUS_Process(st->rxBuffer + 1, st->bytes_received - 3, st->txBuffer, MAXPACKET);
            }
        }
        else
        {
            st->responseLength = 0;
        }
		if(st->responseLength > 0)
        {
            HAL_UART_Transmit_IT(&modbus_uart, st->txBuffer, st->responseLength);
            st->responseLength = 0;
        }
        else
        {
            HAL_UART_Receive_IT(&modbus_uart, st->rxBuffer, MAXPACKET);
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

#define CONTEXT_READ_REG(ctx)    ((enum EnfieldReadRegister)(((uint32_t)ctx)&0x000000FF))
#define CONTEXT_WRITE_REG(ctx) ((enum EnfieldWriteRegister)((((uint32_t)ctx)&0x0000FF00)>>8))
#define CONTEXT_JOINT(ctx)               ((enum JointIndex)((((uint32_t)ctx)&0xC0000000)>>30))

int MODBUS_ReadEnfieldHoldingRegister(void *ctx, uint16_t *v)
{
    osEvent evt;
    struct EnfieldRequest *req;
    struct EnfieldResponse *resp;

    enum JointIndex j = CONTEXT_JOINT(ctx);
    enum EnfieldReadRegister reg = CONTEXT_READ_REG(ctx);

    int ret;

    if(!Enfield_IsValidReadRegister(reg))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    if(j<0 || j>=JOINT_COUNT)
    {
        return ILLEGAL_DATA_ADDRESS;
    }

    req = Enfield_AllocRequest(j);
    req->r = reg;
    req->write = false;
    req->responseQ = enfieldQ;
    req->response = osMailAlloc(enfieldQ, 0);
    Enfield_Request(req);
    ret = SLAVE_DEVICE_FAILURE;
    evt = osMailGet(enfieldQ, osWaitForever);
    if(evt.status == osEventMail)
    {
       resp = evt.value.p;
       *v = resp->value;
       ret = 0;
       osMailFree(enfieldQ, resp);
    }
    else
    {
      // ???
        while(1);
    }
    return ret;
}

int MODBUS_WriteEnfieldHoldingRegister(void *ctx, uint16_t v)
{
    (void)ctx;
    osEvent evt;
    struct EnfieldRequest *req;
    struct EnfieldResponse *resp;

    enum JointIndex j = CONTEXT_JOINT(ctx);
    enum EnfieldWriteRegister reg = CONTEXT_WRITE_REG(ctx);

    int ret;

    if(!Enfield_IsValidWriteRegister(reg))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    if(j<0 || j>=JOINT_COUNT)
    {
        return ILLEGAL_DATA_ADDRESS;
    }

    req = Enfield_AllocRequest(j);
    req->w = reg;
    req->write = true;
    req->value = v;
    req->responseQ = enfieldQ;
    req->response = osMailAlloc(enfieldQ, 0);
    Enfield_Request(req);
    evt = osMailGet(enfieldQ, 100);
    ret = SLAVE_DEVICE_FAILURE;
    if(evt.status == osEventMail)
    {
        resp = evt.value.p;
        ret = resp->err;
        osMailFree(enfieldQ, resp);
    }
    else
    {
        while(1);
    }
    return ret;
}

int MODBUS_ReadEnfieldCoil(void *ctx, bool *v)
{
    uint16_t hv;
    int err = MODBUS_ReadEnfieldHoldingRegister(ctx, &hv);
    *v = (hv == 1);
    return err;
}

int MODBUS_WriteEnfieldCoil(void *ctx, bool v)
{
    uint16_t hv = v ? 0x0001 : 0x0000;
    return MODBUS_WriteEnfieldHoldingRegister(ctx, hv);
}
