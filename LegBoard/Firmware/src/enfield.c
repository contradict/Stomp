#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "enfield.h"
#include "modbus.h"
#include "enfield_uart.h"
#include "export/joint.h"
#include "status_led.h"

#define ENFIELD_BAUD 115200

#define ENFIELD_OK             0
#define ENFIELD_TXFAIL        -1
#define ENFIELD_TXTO          -2
#define ENFIELD_RXFAIL        -3
#define ENFIELD_RXTO          -4
#define ENFIELD_CRCFAIL       -5
#define ENFIELD_WRITEMISMATCH -6

const enum EnfieldWriteRegister EnfieldValidWriteRegister[] = {
    1, 2, 8, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 26, 27, 28, 33, 89, 88,
    224, 225};

const enum EnfieldReadRegister EnfieldValidReadRegister[] = {
    112, 113, 119, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 137, 138,
    139, 144, 145, 146, 147, 148, 152, 153, 154, 155, 158, 161, 162, 163, 164};

enum EnfieldInterruptSignal {
    ENFIELD_RX_COMPLETE = 1,
    ENFIELD_TX_COMPLETE = 2,
    ENFIELD_ERROR = 4,
    ENFIELD_SIGNAL_ALL = 0x07
};

enum EnfieldThreadState {
    StStart,            // Initial state
    StSetZero,          // Set all gains to zero
    StGetCurrent,       // Retrieve current position
    StSetCommandValue,  // Set digital command to current feedback
    StSetCommandSource, // Set command source to digital
    StWaitRequest,      // Wait sample_period for a command, execute if recieved
    StExecuteRequest,   // Execute command
    StUpdate            // smaple period timeout, write position and read pressure
};

struct EnfieldContext
{
    enum JointIndex joint;
    enum EnfieldThreadState state;
    bool successfulInit;
    UART_HandleTypeDef *uart;
    osThreadId thread;
    uint16_t BaseEndPressure;
    uint16_t RodEndPressure;
    uint16_t DigitalCommand;
    uint16_t FeedbackPosition;
    osMailQId commandQ;
    uint8_t txpkt[8];
    uint8_t rxpkt[6];
    struct EnfieldRequest *req;
    struct EnfieldResponse *resp;
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
    .sample_period = 15,
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

    for(enum JointIndex j=0; j<JOINT_COUNT; j++)
    {
        enfield_context[j].uart = &enfield_uart[j];
        enfield_context[j].joint = j;
    }

    enfield_context[JOINT_CURL].thread = osThreadCreate(osThread(enfield_curl_thread), &enfield_context[JOINT_CURL]);
    enfield_context[JOINT_CURL].commandQ = osMailCreate(osMailQ(curlcommand), enfield_context[JOINT_CURL].thread);

    enfield_context[JOINT_SWING].thread = osThreadCreate(osThread(enfield_swing_thread), &enfield_context[JOINT_SWING]);
    enfield_context[JOINT_SWING].commandQ = osMailCreate(osMailQ(swingcommand), enfield_context[JOINT_SWING].thread);

    enfield_context[JOINT_LIFT].thread = osThreadCreate(osThread(enfield_lift_thread), &enfield_context[JOINT_LIFT]);
    enfield_context[JOINT_LIFT].commandQ = osMailCreate(osMailQ(liftcommand), enfield_context[JOINT_LIFT].thread);
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

int Enfield_ReadDigitalCommand(void *ctx, uint16_t *v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    *v = enfield_context[joint].DigitalCommand;
    return 0;
}

int Enfield_ReadFeedbackPosition(void *ctx, uint16_t *v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    *v = enfield_context[joint].FeedbackPosition;
    return 0;
}

int Enfield_WriteDigitalCommand(void *ctx, uint16_t v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    enfield_context[joint].DigitalCommand = v;
    return 0;
}

bool Enfield_IsValidReadRegister(enum EnfieldReadRegister r)
{
    for(size_t i=0; i < sizeof(EnfieldValidReadRegister) / sizeof(EnfieldValidReadRegister[0]); i++)
    {
        if( r == EnfieldValidReadRegister[i])
            return true;
    }
    return false;
}

bool Enfield_IsValidWriteRegister(enum EnfieldWriteRegister r)
{
    for(size_t i=0; i < sizeof(EnfieldValidWriteRegister) / sizeof(EnfieldValidWriteRegister[0]); i++)
    {
        if( r == EnfieldValidWriteRegister[i])
            return true;
    }
    return false;
}

#define CLIP(x, mn, mx) (((x)<(mn))?(mn):(((x)>(mx))?(mx):(x)))

void Enfield_SetCommand(uint16_t command[JOINT_COUNT])
{
    for(int j=0;j<3;j++)
        enfield_context[j].DigitalCommand = CLIP(command[j], 0, 4095);
}

static void Curl_UART_Init()
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

static void Lift_UART_Init()
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

static void Swing_UART_Init()
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
    struct EnfieldContext *st = (struct EnfieldContext *)arg;
    st->state = StStart;
    uint16_t read_data, write_data;
    int err, errs[4];
    while(1)
    {
        switch(st->state)
        {
            case StStart:
                st->state = StSetZero;
                st->successfulInit = false;
                break;
            case StSetZero:
                LED_SetOne(st->joint, 0, 128);
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
                    st->state = StGetCurrent;
                }
                else
                {
                    st->state = StWaitRequest;
                }
                break;
            case StGetCurrent:
                err = Enfield_Get(st, ReadFeedbackPosition, &st->FeedbackPosition);
                if(ENFIELD_OK == err)
                {
                    st->state = StSetCommandValue;
                }
                else
                {
                    st->state = StWaitRequest;
                }
                break;
            case StSetCommandValue:
                err = Enfield_Write(st, SetDigitalCommand, &st->FeedbackPosition);
                if(ENFIELD_OK == err)
                {
                    st->DigitalCommand = st->FeedbackPosition;
                    st->state = StSetCommandSource;
                }
                else
                {
                    st->state = StWaitRequest;
                }
                break;
            case StSetCommandSource:
                write_data = COMMAND_SOURCE_DIGITAL;
                err = Enfield_Write(st, SetCommandSource, &write_data);
                if(ENFIELD_OK == err)
                {
                    LED_SetOne(st->joint, 0, 0);
                    st->successfulInit = true;
                }
                st->state = StWaitRequest;
                break;
            case StWaitRequest:
                evt = osMailGet(st->commandQ, enfield_parameters.sample_period);
                if(evt.status == osEventTimeout)
                {
                    st->state = st->successfulInit ? StUpdate : StSetZero;
                }
                else if(evt.status == osEventMail)
                {
                    st->req = (struct EnfieldRequest *)evt.value.p;
                    st->resp = st->req->response;
                    st->state = StExecuteRequest;
                }
                break;
            case StExecuteRequest:
                LED_BlinkOne(st->joint, 2, 255, 20);
                if(st->req->write)
                {
                    st->resp->value = st->req->value;
                    st->resp->err = Enfield_Write(st, st->req->w, &st->resp->value);
                }
                else
                {
                    st->resp->err = Enfield_Get(st, st->req->r, &st->resp->value);
                }
                osMailPut(st->req->responseQ, st->resp);
                osMailFree(st->commandQ, st->req);
                st->state = StWaitRequest;
                break;
            case StUpdate:
                LED_SetOne(st->joint, 2, 64);
                errs[0] = Enfield_Get(st, ReadBaseEndPressure, &read_data);
                if(ENFIELD_OK == errs[0])
                {
                    st->BaseEndPressure = read_data;
                }
                errs[1] = Enfield_Get(st, ReadRodEndPressure, &read_data);
                if(ENFIELD_OK == errs[1])
                {
                    st->RodEndPressure = read_data;
                }
                errs[2] = Enfield_Get(st, ReadFeedbackPosition, &read_data);
                if(ENFIELD_OK == errs[1])
                {
                    st->FeedbackPosition = read_data;
                }
                write_data = st->DigitalCommand;
                errs[3] = Enfield_Write(st, SetDigitalCommand, &write_data);
                if(errs[0] || errs[1] || errs[2])
                {
                    err = 1;
                }
                st->state = StWaitRequest;
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
    // don't wait for unacked commands
    if((r == SetZeroGains) || (r == SetSaveConfiguration))
    {
        osDelay(500);
        return Enfield_WaitTransmit(enf);
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
    return Enfield_Transfer(enf, r, v);
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
