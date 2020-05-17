#include "stm32f7xx_hal.h"
#include "modbus_uart.h"
#include "enfield_uart.h"

extern void MODBUS_TxCplt(UART_HandleTypeDef *huart);
extern void MODBUS_RxCplt(UART_HandleTypeDef *huart);
extern void MODBUS_RxTo(UART_HandleTypeDef *huart);
extern void MODBUS_UARTError(UART_HandleTypeDef *huart);
extern void Enfield_TxCplt(UART_HandleTypeDef *huart);
extern void Enfield_RxCplt(UART_HandleTypeDef *huart);
extern void Enfield_UARTError(UART_HandleTypeDef *huart);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        MODBUS_TxCplt(huart);
    } else
    {
        Enfield_TxCplt(huart);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        MODBUS_RxCplt(huart);
    } else
    {
        Enfield_RxCplt(huart);
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
        Enfield_UARTError(huart);
    }
}
