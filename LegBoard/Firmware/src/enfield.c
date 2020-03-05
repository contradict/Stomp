#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "enfield.h"
#include "modbus.h"
#include "enfield_uart.h"
#include "joint.h"

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
    ENFIELD_MESSAGE = 8,
    ENFIELD_SIGNAL_ALL = 0x0f
};

enum EnfieldThreadState {
    Start,
    SetZero,
    WaitRequest,
    ExecuteRequest,
    Update
};

struct EnfieldContext
{
    UART_HandleTypeDef *uart;
    osThreadId thread;
    uint16_t BaseEndPressure;
    uint16_t RodEndPressure;
    uint16_t DigitalCommand;
    osMailQId commandQ;
};

struct EnfieldParameters {
    uint16_t sample_period;
    uint16_t transmit_timeout;
    uint16_t data_timeout;
};

static void Enfield_UART_Init();
static void Enfield_Thread(const void *arg);
static int Enfield_SendCommand(struct EnfieldContext *enf, uint8_t r, uint16_t v);
static int Enfield_WaitTransmit(void);
static int Enfield_ReceiveResponse(struct EnfieldContext *enf, uint8_t *pkt);
static int Enfield_WaitReceive(uint8_t *pkt, uint16_t *v);
static int Enfield_Get(struct EnfieldContext *enf, enum EnfieldReadRegister r, uint16_t *v);
static int Enfield_Write(struct EnfieldContext *enf, enum EnfieldWriteRegister r, uint16_t *v);

static struct EnfieldParameters parameters __attribute__ ((section (".storage.enfield"))) = {
    .sample_period = 50,
    .transmit_timeout = 2,
    .data_timeout = 10
};

UART_HandleTypeDef enfield_uart[JOINT_COUNT];
static struct EnfieldContext enfield_context[JOINT_COUNT];
osThreadDef(enfield_thread, Enfield_Thread, osPriorityAboveNormal, 3, configMINIMAL_STACK_SIZE);

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

    enfield_context[JOINT_CURL].thread = osThreadCreate(osThread(enfield_thread), &enfield_context[JOINT_CURL]);
    enfield_context[JOINT_CURL].commandQ = osMailCreate(osMailQ(curlcommand), enfield_context[JOINT_CURL].thread);
    enfield_context[JOINT_SWING].thread = osThreadCreate(osThread(enfield_thread), &enfield_context[JOINT_SWING]);
    enfield_context[JOINT_SWING].commandQ = osMailCreate(osMailQ(curlcommand), enfield_context[JOINT_SWING].thread);
    enfield_context[JOINT_LIFT].thread = osThreadCreate(osThread(enfield_thread), &enfield_context[JOINT_LIFT]);
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

void Curl_UART_Init()
{
    enfield_uart[JOINT_CURL].Instance = CURL_UART_Instance;
    enfield_uart[JOINT_CURL].Init.BaudRate = 57600;
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
    enfield_uart[JOINT_LIFT].Init.BaudRate = 57600;
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
    enfield_uart[JOINT_SWING].Init.BaudRate = 57600;
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
    uint16_t dummy_command;
    while(1)
    {
        switch(state)
        {
            case Start:
                state = SetZero;
                break;
            case SetZero:
                dummy_command = 0x0000;
                if(ENFIELD_OK == Enfield_Write(st, SetZeroGains, &dummy_command))
                {
                    state = WaitRequest;
                }
                break;
            case WaitRequest:
                evt = osMailGet(st->commandQ, parameters.sample_period);
                if(evt.status & osEventTimeout)
                {
                    state = Update;
                }
                else if(evt.status & osEventMail)
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
                break;
            case Update:
                Enfield_Get(st, ReadBaseEndPressure, &(st->BaseEndPressure));
                Enfield_Get(st, ReadRodEndPressure, &(st->RodEndPressure));
                dummy_command = st->DigitalCommand;
                Enfield_Write(st, SetDigitalCommand, &dummy_command);
                state = WaitRequest;
                break;
        }
    }
}

static int Enfield_SendCommand(struct EnfieldContext *enf, uint8_t r, uint16_t v)
{
    uint8_t pkt[8] = {'$', 'C', r, 0x11, 0x11, '#', 0x00, 0x00};

    *(uint16_t *)(pkt + 3) = v;
    *(uint16_t *)&pkt[6] = MODBUS_crc(pkt, 6);

    if(0 == HAL_UART_Transmit_IT(enf->uart, pkt, 8))
    {
        return ENFIELD_OK;
    }
    else
    {
        return ENFIELD_TXFAIL;
    }
}

static int Enfield_WaitTransmit(void)
{
    osEvent evt;
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, parameters.transmit_timeout);
    if(!(evt.status == osEventSignal && (evt.value.signals & ENFIELD_TX_COMPLETE)))
    {
        return ENFIELD_TXTO;
    }
    return ENFIELD_OK;
}

static int Enfield_ReceiveResponse(struct EnfieldContext *enf, uint8_t *pkt)
{
    if(HAL_OK == HAL_UART_Receive_IT(enf->uart, pkt, 6))
    {
        return ENFIELD_OK;
    }
    else
    {
        return ENFIELD_RXFAIL;
    }
}

static int Enfield_WaitReceive(uint8_t *pkt, uint16_t *v)
{
    osEvent evt;
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, parameters.data_timeout);
    if(!(evt.status == osEventSignal && (evt.value.signals & ENFIELD_RX_COMPLETE)))
    {
        return ENFIELD_RXTO;
    }
    if(MODBUS_crc(pkt, 6) == 0)
    {
        *v = *(uint16_t *)(pkt+1);
        return ENFIELD_OK;
    }
    return ENFIELD_CRCFAIL;
}

static int Enfield_Transfer(struct EnfieldContext *enf,  uint8_t r, uint16_t *v)
{
    uint8_t rxpkt[6];
    int err;
    err = Enfield_SendCommand(enf, r, *v);
    if(err != ENFIELD_OK)
    {
        return err;
    }
    err = Enfield_WaitTransmit();
    if(err != ENFIELD_OK)
    {
        return err;
    }
    err = Enfield_ReceiveResponse(enf, rxpkt);
    if(err != ENFIELD_OK)
    {
        return err;
    }
    return Enfield_WaitReceive(rxpkt, v);
}

static int Enfield_Get(struct EnfieldContext *enf, enum EnfieldReadRegister r, uint16_t *v)
{
    uint8_t err;
    err = Enfield_Transfer(enf, r, v);
    if(err != ENFIELD_OK)
    {
        return err;
    }
    return ENFIELD_OK;
}

static int Enfield_Write(struct EnfieldContext *enf, enum EnfieldWriteRegister r, uint16_t *v)
{
    uint16_t vcopy = *v;
    uint8_t err;
    err = Enfield_Transfer(enf, r, v);
    if(err != ENFIELD_OK)
    {
        return err;
    }
    if(vcopy != *v)
    {
        return ENFIELD_WRITEMISMATCH;
    }
    return ENFIELD_OK;
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
