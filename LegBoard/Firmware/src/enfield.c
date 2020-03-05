#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "enfield.h"
#include "modbus.h"
#include "enfield_uart.h"
#include "main.h"

enum EnfieldInterruptSignal {
    ENFIELD_RX_COMPLETE = 1,
    ENFIELD_TX_COMPLETE = 2,
    ENFIELD_ERROR = 4,
    ENFIELD_SIGNAL_ALL = 7
};

struct EnfieldState
{
    UART_HandleTypeDef *uart;
    osThreadId thread;
    uint16_t FeedbackInput;
    uint16_t BaseEndPressure;
    uint16_t RodEndPressure;
};

static void Enfield_Thread(const void *arg);
static int Enfield_Get(struct EnfieldState *enf, enum EnfieldReadRegister r, uint16_t *v);
static int Enfield_Write(struct EnfieldState *enf, enum EnfieldReadRegister r, uint16_t v);

UART_HandleTypeDef enfield_uart[NJOINTS];
static struct EnfieldState enfield_state[NJOINTS];
osThreadDef(enfield_thread, Enfield_Thread, osPriorityAboveNormal, 3, configMINIMAL_STACK_SIZE);

void Enfield_Init(void)
{
    Enfield_UART_Init();

    bzero(&enfield_state, sizeof(enfield_state));

    enfield_state[CURL].uart = &enfield_uart[CURL];
    enfield_state[SWING].uart = &enfield_uart[SWING];
    enfield_state[LIFT].uart = &enfield_uart[LIFT];

    enfield_state[CURL].thread = osThreadCreate(osThread(enfield_thread), &enfield_state[CURL]);
    enfield_state[SWING].thread = osThreadCreate(osThread(enfield_thread), &enfield_state[SWING]);
    enfield_state[LIFT].thread = osThreadCreate(osThread(enfield_thread), &enfield_state[LIFT]);
}

void Curl_UART_Init()
{
    enfield_uart[CURL].Instance = CURL_UART_Instance;
    enfield_uart[CURL].Init.BaudRate = 57600;
    enfield_uart[CURL].Init.WordLength = UART_WORDLENGTH_8B;
    enfield_uart[CURL].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    enfield_uart[CURL].Init.Mode = UART_MODE_TX_RX;
    enfield_uart[CURL].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    enfield_uart[CURL].Init.OverSampling = UART_OVERSAMPLING_16;
    enfield_uart[CURL].Init.Parity = UART_PARITY_NONE;
    enfield_uart[CURL].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&enfield_uart[CURL]);
}

void Lift_UART_Init()
{
    enfield_uart[LIFT].Instance = LIFT_UART_Instance;
    enfield_uart[LIFT].Init.BaudRate = 57600;
    enfield_uart[LIFT].Init.WordLength = UART_WORDLENGTH_8B;
    enfield_uart[LIFT].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    enfield_uart[LIFT].Init.Mode = UART_MODE_TX_RX;
    enfield_uart[LIFT].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    enfield_uart[LIFT].Init.OverSampling = UART_OVERSAMPLING_16;
    enfield_uart[LIFT].Init.Parity = UART_PARITY_NONE;
    enfield_uart[LIFT].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&enfield_uart[LIFT]);
}

void Swing_UART_Init()
{
    enfield_uart[SWING].Instance = SWING_UART_Instance;
    enfield_uart[SWING].Init.BaudRate = 57600;
    enfield_uart[SWING].Init.WordLength = UART_WORDLENGTH_8B;
    enfield_uart[SWING].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    enfield_uart[SWING].Init.Mode = UART_MODE_TX_RX;
    enfield_uart[SWING].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    enfield_uart[SWING].Init.OverSampling = UART_OVERSAMPLING_16;
    enfield_uart[SWING].Init.Parity = UART_PARITY_NONE;
    enfield_uart[SWING].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&enfield_uart[SWING]);
}

void Enfield_UART_Init()
{
    Curl_UART_Init();
    Lift_UART_Init();
    Swing_UART_Init();
}

void Enfield_Thread(const void *arg)
{
    struct EnfieldState *st = (struct EnfieldState *)arg;
    while(1)
    {
        Enfield_Get(st, ReadFeedbackInput, &(st->FeedbackInput));
        Enfield_Get(st, ReadBaseEndPressure, &(st->FeedbackInput));
        Enfield_Get(st, ReadRodEndPressure, &(st->FeedbackInput));
    }
}

static int Enfield_Get(struct EnfieldState *enf, enum EnfieldReadRegister r, uint16_t *v)
{
    osEvent evt;
    uint8_t pkt[8] = {'$', 'C', r, 0x11, 0x11, '#', 0x00, 0x00};

    *(uint16_t *)&pkt[6] = MODBUS_crc(pkt, 6);

    HAL_UART_Transmit_IT(enf->uart, pkt, 8);
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, 100);
    if(!(evt.status == osEventSignal && (evt.value.signals & ENFIELD_TX_COMPLETE)))
    {
        return -1;
    }
    HAL_UART_Receive_IT(enf->uart, pkt, 6);
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, 100);
    if(!(evt.status == osEventSignal && (evt.value.signals & ENFIELD_RX_COMPLETE)))
    {
        return -1;
    }
    if(MODBUS_crc(pkt, 6) == 0)
    {
        *v = *(uint16_t *)(pkt+1);
        return 0;
    }
    return -2;
}

static int Enfield_Write(struct EnfieldState *enf, enum EnfieldReadRegister r, uint16_t v)
{
    osEvent evt;
    uint8_t pkt[8] = {'$', 'C', r, 0x0, 0x0, '#', 0x00, 0x00};

    *(uint16_t *)(pkt + 3) = v;

    *(uint16_t *)&pkt[6] = MODBUS_crc(pkt, 6);

    HAL_UART_Transmit_IT(enf->uart, pkt, 8);
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, 100);
    if(!(evt.status == osEventSignal && (evt.value.signals & ENFIELD_TX_COMPLETE)))
    {
        return -1;
    }
    HAL_UART_Receive_IT(enf->uart, pkt, 6);
    evt = osSignalWait(ENFIELD_SIGNAL_ALL, 100);
    if(!(evt.status == osEventSignal && (evt.value.signals & ENFIELD_RX_COMPLETE)))
    {
        return -1;
    }
    if(MODBUS_crc(pkt, 6) == 0)
    {
        if(*(uint16_t *)(pkt + 3) == v)
        {
            return 0;
        }
        else
        {
            return -3;
        }
    }
    return -2;
}

void Enfield_RxCplt(UART_HandleTypeDef *huart)
{
    for(int i=0; i<3; i++)
    {
        if(enfield_state[i].uart == huart)
        {
            osSignalSet(enfield_state[i].thread, ENFIELD_RX_COMPLETE);
            break;
        }
    }
}

void Enfield_TxCplt(UART_HandleTypeDef *huart)
{
    for(int i=0; i<3; i++)
    {
        if(enfield_state[i].uart == huart)
        {
            osSignalSet(enfield_state[i].thread, ENFIELD_TX_COMPLETE);
            break;
        }
    }
}

void Enfield_UARTError(UART_HandleTypeDef *huart)
{
    for(int i=0; i<3; i++)
    {
        if(enfield_state[i].uart == huart)
        {
            osSignalSet(enfield_state[i].thread, ENFIELD_ERROR);
            break;
        }
    }
}
