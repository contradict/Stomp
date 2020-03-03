#include "stm32f7xx_hal.h"
#include "stm32f7xx_ll_usart.h"
#include "modbus_uart.h"
#include "servo_uart.h"
#include "main.h"

extern void MODBUS_TxCplt(UART_HandleTypeDef *huart);
extern void MODBUS_RxCplt(UART_HandleTypeDef *huart);
extern void MODBUS_RxTo(UART_HandleTypeDef *huart);
extern void MODBUS_UARTError(UART_HandleTypeDef *huart);
extern void Servo_TxCplt(UART_HandleTypeDef *huart);
extern void Servo_RxCplt(UART_HandleTypeDef *huart);
extern void Servo_UARTError(UART_HandleTypeDef *huart);

UART_HandleTypeDef modbus_uart;
UART_HandleTypeDef servo_uart[NJOINTS];

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

void Curl_UART_Init()
{
    servo_uart[CURL].Instance = CURL_UART_Instance;
    servo_uart[CURL].Init.BaudRate = 57600;
    servo_uart[CURL].Init.WordLength = UART_WORDLENGTH_8B;
    servo_uart[CURL].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    servo_uart[CURL].Init.Mode = UART_MODE_TX_RX;
    servo_uart[CURL].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    servo_uart[CURL].Init.OverSampling = UART_OVERSAMPLING_16;
    servo_uart[CURL].Init.Parity = UART_PARITY_NONE;
    servo_uart[CURL].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&servo_uart[CURL]);
}

void Lift_UART_Init()
{
    servo_uart[LIFT].Instance = LIFT_UART_Instance;
    servo_uart[LIFT].Init.BaudRate = 57600;
    servo_uart[LIFT].Init.WordLength = UART_WORDLENGTH_8B;
    servo_uart[LIFT].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    servo_uart[LIFT].Init.Mode = UART_MODE_TX_RX;
    servo_uart[LIFT].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    servo_uart[LIFT].Init.OverSampling = UART_OVERSAMPLING_16;
    servo_uart[LIFT].Init.Parity = UART_PARITY_NONE;
    servo_uart[LIFT].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&servo_uart[LIFT]);
}

void Swing_UART_Init()
{
    servo_uart[SWING].Instance = SWING_UART_Instance;
    servo_uart[SWING].Init.BaudRate = 57600;
    servo_uart[SWING].Init.WordLength = UART_WORDLENGTH_8B;
    servo_uart[SWING].Init.HwFlowCtl = UART_HWCONTROL_NONE;
    servo_uart[SWING].Init.Mode = UART_MODE_TX_RX;
    servo_uart[SWING].Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    servo_uart[SWING].Init.OverSampling = UART_OVERSAMPLING_16;
    servo_uart[SWING].Init.Parity = UART_PARITY_NONE;
    servo_uart[SWING].Init.StopBits = UART_STOPBITS_1;
    HAL_UART_Init(&servo_uart[SWING]);
}

void Servo_UART_Init()
{
    Curl_UART_Init();
    Lift_UART_Init();
    Swing_UART_Init();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        MODBUS_TxCplt(huart);
    } else
    {
        Servo_TxCplt(huart);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        MODBUS_RxCplt(huart);
    } else
    {
        Servo_RxCplt(huart);
    }
}

void HAL_UART_RxToCallback(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        MODBUS_RxTo(huart);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        MODBUS_UARTError(huart);
    } else
    {
        Servo_UARTError(huart);
    }
}
