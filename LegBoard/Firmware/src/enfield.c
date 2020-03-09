#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "enfield.h"
#include "modbus.h"
#include "enfield_uart.h"
#include "joint.h"

#define ENFIELD_BAUD 115200

#define ENFIELD_OK             0
#define ENFIELD_TXFAIL        -1
#define ENFIELD_TXTO          -2
#define ENFIELD_RXFAIL        -3
#define ENFIELD_RXTO          -4
#define ENFIELD_CRCFAIL       -5
#define ENFIELD_WRITEMISMATCH -6

enum EnfieldInterruptSignal {
    ENFIELD_RX_COMPLETE = 1,
    ENFIELD_TX_COMPLETE = 2,
    ENFIELD_ERROR = 4,
    ENFIELD_SIGNAL_ALL = 0x07
};

enum EnfieldThreadState {
    Start,            // Initial state
    SetZero,          // Set all gains to zero
    GetCurrent,       // Retrieve current position
    SetCommand,       // Set command source to digital
    WaitRequest,      // Wait sample_period for a command, execute if recieved
    ExecuteRequest,   // Execute command
    Update            // smaple period timeout, write position and read pressure
};

struct EnfieldContext
{
    UART_HandleTypeDef *uart;
    osThreadId thread;
    uint16_t BaseEndPressure;
    uint16_t RodEndPressure;
    uint16_t DigitalCommand;
    osMailQId commandQ;
    uint8_t txpkt[8];
    uint8_t rxpkt[6];
};

struct EnfieldParameters {
    uint16_t sample_period;
    uint16_t transmit_timeout;
    uint16_t data_timeout;
};

static void Enfield_UART_Init();
static void Enfield_Thread(const void *arg);
static int Enfield_SendCommand(struct EnfieldContext *enf, uint8_t r, uint16_t v);
static int Enfield_WaitTransmit(struct EnfieldContext *enf);
static int Enfield_ReceiveResponse(struct EnfieldContext *enf);
static int Enfield_WaitReceive(struct EnfieldContext *enf, uint16_t *v);
static int Enfield_Get(struct EnfieldContext *enf, enum EnfieldReadRegister r, uint16_t *v);
static int Enfield_Write(struct EnfieldContext *enf, enum EnfieldWriteRegister r, uint16_t *v);

static struct EnfieldParameters enfield_parameters __attribute__ ((section (".storage.enfield"))) = {
    .sample_period = 50,
    .transmit_timeout = 2,
    .data_timeout = 10
};

UART_HandleTypeDef enfield_uart[JOINT_COUNT];
static struct EnfieldContext enfield_context[JOINT_COUNT];
osThreadDef(enfield_curl_thread, Enfield_Thread, osPriorityAboveNormal, 1, configMINIMAL_STACK_SIZE);
osThreadDef(enfield_swing_thread, Enfield_Thread, osPriorityAboveNormal, 1, configMINIMAL_STACK_SIZE);
osThreadDef(enfield_lift_thread, Enfield_Thread, osPriorityAboveNormal, 1, configMINIMAL_STACK_SIZE);

osMailQDef(curlcommand, 32, struct EnfieldRequest);
osMailQDef(swingcommand, 32, struct EnfieldRequest);
osMailQDef(liftcommand, 32, struct EnfieldRequest);

void Enfield_Init(void)
{
    Enfield_UART_Init();

    bzero(&enfield_context, sizeof(enfield_context));

    enfield_context[JOINT_CURL].uart = &enfield_uart[JOINT_CURL];
    enfield_context[JOINT_SWING].uart = &enfield_uart[JOINT_SWING];
    enfield_context[JOINT_LIFT].uart = &enfield_uart[JOINT_LIFT];

    enfield_context[JOINT_CURL].thread = osThreadCreate(osThread(enfield_curl_thread), &enfield_context[JOINT_CURL]);
    enfield_context[JOINT_CURL].commandQ = osMailCreate(osMailQ(curlcommand), enfield_context[JOINT_CURL].thread);
    enfield_context[JOINT_SWING].thread = osThreadCreate(osThread(enfield_swing_thread), &enfield_context[JOINT_SWING]);
    enfield_context[JOINT_SWING].commandQ = osMailCreate(osMailQ(curlcommand), enfield_context[JOINT_SWING].thread);
    enfield_context[JOINT_LIFT].thread = osThreadCreate(osThread(enfield_lift_thread), &enfield_context[JOINT_LIFT]);
    enfield_context[JOINT_LIFT].commandQ = osMailCreate(osMailQ(curlcommand), enfield_context[JOINT_LIFT].thread);
}

struct EnfieldRequest * Enfield_AllocRequest(enum JointIndex joint)
{
    struct EnfieldRequest *req = osMailAlloc(enfield_context[joint].commandQ, 0);
    req->joint = joint;
    return req;
}

void Enfield_Request(struct EnfieldRequest *req)
{
    osMailPut(enfield_context[req->joint].commandQ, req);
}

int Enfield_ReadBaseEndPresure(void *ctx, uint16_t *v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    *v = enfield_context[joint].BaseEndPressure;
    return 0;
}

int Enfield_ReadRodEndPresure(void *ctx, uint16_t *v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    *v = enfield_context[joint].RodEndPressure;
    return 0;
}

void Curl_UART_Init()
{
    enfield_uart[JOINT_CURL].Instance = CURL_UART_Instance;
    enfield_uart[JOINT_CURL].Init.BaudRate = ENFIELD_BAUD;
    enfield_uart[JOINT_CURL].Init.WordLength = UART_WORDLENGTH_8B;
    enfield_uart[JOINT_CURL].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    enfield_uart[JOINT_CURL].Init.Mode = UART_MODE_TX_RX;
    enfield_uart[JOINT_CURL].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    enfield_uart[JOINT_CURL].Init.OverSampling = UART_OVERSAMPLING_16;
    enfield_uart[JOINT_CURL].Init.Parity = UART_PARITY_NONE;
    enfield_uart[JOINT_CURL].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&enfield_uart[JOINT_CURL]);
}

void Lift_UART_Init()
{
    enfield_uart[JOINT_LIFT].Instance = LIFT_UART_Instance;
    enfield_uart[JOINT_LIFT].Init.BaudRate = ENFIELD_BAUD;
    enfield_uart[JOINT_LIFT].Init.WordLength = UART_WORDLENGTH_8B;
    enfield_uart[JOINT_LIFT].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    enfield_uart[JOINT_LIFT].Init.Mode = UART_MODE_TX_RX;
    enfield_uart[JOINT_LIFT].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    enfield_uart[JOINT_LIFT].Init.OverSampling = UART_OVERSAMPLING_16;
    enfield_uart[JOINT_LIFT].Init.Parity = UART_PARITY_NONE;
    enfield_uart[JOINT_LIFT].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&enfield_uart[JOINT_LIFT]);
}

void Swing_UART_Init()
{
    enfield_uart[JOINT_SWING].Instance = SWING_UART_Instance;
    enfield_uart[JOINT_SWING].Init.BaudRate = ENFIELD_BAUD;
    enfield_uart[JOINT_SWING].Init.WordLength = UART_WORDLENGTH_8B;
    enfield_uart[JOINT_SWING].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    enfield_uart[JOINT_SWING].Init.Mode = UART_MODE_TX_RX;
    enfield_uart[JOINT_SWING].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    enfield_uart[JOINT_SWING].Init.OverSampling = UART_OVERSAMPLING_16;
    enfield_uart[JOINT_SWING].Init.Parity = UART_PARITY_NONE;
    enfield_uart[JOINT_SWING].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&enfield_uart[JOINT_SWING]);
}

void Enfield_UART_Init()
{
    Curl_UART_Init();
    Lift_UART_Init();
    Swing_UART_Init();
}

void Enfield_Thread(const void *arg)
{
    osEvent evt;
    enum EnfieldThreadState state = Start;
    struct EnfieldContext *st = (struct EnfieldContext *)arg;
    struct EnfieldRequest *req;
    uint16_t write_data;
    int err;
    while(1)
    {
        switch(state)
        {
            case Start:
                state = SetZero;
                break;
            case SetZero:
                write_data = 0x0000;
                // Command has no response, ignore RX error
                err = Enfield_Write(st, SetZeroGains, &write_data);
                if(err == ENFIELD_RXTO)
                {
                    err = 0;
                }
                write_data = 0x0000;
                err  = Enfield_Write(st, SetProportionalGain, &write_data);
                write_data = 0x0000;
                err += Enfield_Write(st, SetDerivativeGain, &write_data);
                if(ENFIELD_OK == err)
                {
                    state = GetCurrent;
                }
                else
                {
                    osDelay(50);
                }
                break;
            case GetCurrent:
                err = Enfield_Get(st, ReadFeedbackPosition, &st->DigitalCommand);
                if(ENFIELD_OK == err)
                {
                    state = SetCommand;
                }
                else
                {
                    osDelay(50);
                }
                break;
            case SetCommand:
                write_data = COMMAND_SOURCE_DIGITAL;
                err = Enfield_Write(st, SetCommandSource, &write_data);
                if(ENFIELD_OK == err)
                {
                    LED_SetOne(st->joint, 0, 0);
                    state = WaitRequest;
                }
                else
                {
                    osDelay(50);
                }
                break;
            case WaitRequest:
                evt = osMailGet(st->commandQ, enfield_parameters.sample_period);
                if(evt.status == osEventTimeout)
                {
                    state = Update;
                }
                else if(evt.status == osEventMail)
                {
                    state = ExecuteRequest;
                }
                break;
            case ExecuteRequest:
                req = (struct EnfieldRequest *)evt.value.p;
                if(req->write)
                {
                    req->response->value = req->value;
                    req->response->err = Enfield_Write(st, req->w, &req->response->value);
                }
                else
                {
                    req->response->err = Enfield_Get(st, req->r, &req->response->value);
                }
                osMailPut(req->responseQ, req->response);
                osMailFree(st->commandQ, req);
                state = Update;
                break;
            case Update:
                Enfield_Get(st, ReadBaseEndPressure, &(st->BaseEndPressure));
                Enfield_Get(st, ReadRodEndPressure, &(st->RodEndPressure));
                write_data = st->DigitalCommand;
                Enfield_Write(st, SetDigitalCommand, &write_data);
                state = WaitRequest;
                break;
        }
    }
}

static int Enfield_SendCommand(struct EnfieldContext *enf, uint8_t r, uint16_t v)
{
    enf->txpkt[0] = '$';
    enf->txpkt[1] = 'C';
    enf->txpkt[2] = r;
    *(uint16_t *)(enf->txpkt + 3) = v;
    enf->txpkt[5] = '#';
    *(uint16_t *)&enf->txpkt[6] = MODBUS_crc(enf->txpkt, 6);

    if(0 == HAL_UART_Transmit_DMA(enf->uart, enf->txpkt, 8))
    {
        return ENFIELD_OK;
    }
    else
    {
        return ENFIELD_TXFAIL;
    }
}

static int Enfield_WaitTransmit(struct EnfieldContext *enf)
{
    osEvent evt;
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, enfield_parameters.transmit_timeout);
    if(evt.status == osEventTimeout)
    {
        HAL_UART_AbortTransmit_IT(enf->uart);
        return ENFIELD_TXTO;
    }
    if((evt.status == osEventSignal) &&
       (evt.value.signals & ENFIELD_TX_COMPLETE))
    {
        return ENFIELD_OK;
    }
    return ENFIELD_TXFAIL;
}

static int Enfield_ReceiveResponse(struct EnfieldContext *enf)
{
    if(HAL_OK == HAL_UART_Receive_DMA(enf->uart, enf->rxpkt, 6))
    {
        return ENFIELD_OK;
    }
    else
    {
        return ENFIELD_RXFAIL;
    }
}

static int Enfield_WaitReceive(struct EnfieldContext *enf, uint16_t *v)
{
    osEvent evt;
    int err;
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, enfield_parameters.data_timeout);
    if(evt.status == osEventTimeout)
    {
        HAL_UART_AbortReceive_IT(enf->uart);
        err = ENFIELD_RXTO;
    }
    else if((evt.status == osEventSignal) &&
       (evt.value.signals & ENFIELD_RX_COMPLETE))
    {
        if(MODBUS_crc(enf->rxpkt, 6) == 0)
        {
            *v = *(uint16_t *)(enf->rxpkt+1);
            err = ENFIELD_OK;
        }
        else
        {
            err = ENFIELD_CRCFAIL;
        }
    }
    else
    {
        err = ENFIELD_RXFAIL;
    }
    return err;
}

static int Enfield_Transfer(struct EnfieldContext *enf,  uint8_t r, uint16_t *v)
{
    int err;
    err = Enfield_SendCommand(enf, r, *v);
    if(err != ENFIELD_OK)
    {
        return err;
    }
    err = Enfield_ReceiveResponse(enf);
    if(err != ENFIELD_OK)
    {
        HAL_UART_AbortTransmit_IT(enf->uart);
        return err;
    }
    err = Enfield_WaitTransmit(enf);
    if(err != ENFIELD_OK)
    {
        HAL_UART_AbortReceive_IT(enf->uart);
        return err;
    }
    err =  Enfield_WaitReceive(enf, v);
    return err;
}

static int Enfield_Get(struct EnfieldContext *enf, enum EnfieldReadRegister r, uint16_t *v)
{
    uint8_t err;
    err = Enfield_Transfer(enf, r, v);
    return err;
}

static int Enfield_Write(struct EnfieldContext *enf, enum EnfieldWriteRegister r, uint16_t *v)
{
    uint8_t err;
    err = Enfield_Transfer(enf, r, v);
    return err;
}

void Enfield_RxCplt(UART_HandleTypeDef *huart)
{
    for(int i=0; i<3; i++)
    {
        if(enfield_context[i].uart == huart)
        {
            osSignalSet(enfield_context[i].thread, ENFIELD_RX_COMPLETE);
            break;
        }
    }
}

void Enfield_TxCplt(UART_HandleTypeDef *huart)
{
    for(int i=0; i<3; i++)
    {
        if(enfield_context[i].uart == huart)
        {
            osSignalSet(enfield_context[i].thread, ENFIELD_TX_COMPLETE);
            break;
        }
    }
}

void Enfield_UARTError(UART_HandleTypeDef *huart)
{
    for(int i=0; i<3; i++)
    {
        if(enfield_context[i].uart == huart)
        {
            osSignalSet(enfield_context[i].thread, ENFIELD_ERROR);
            break;
        }
    }
}
