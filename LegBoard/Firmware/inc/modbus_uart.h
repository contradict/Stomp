#pragma once

#include "stm32f7xx_hal.h"

extern UART_HandleTypeDef modbus_uart;

#define MODBUS_UART_Instance          USART1
#define MODBUS_UART_CLK_ENABLE()      __HAL_RCC_USART1_CLK_ENABLE()
#define MODBUS_UART_FORCE_RESET()     __HAL_RCC_USART1_FORCE_RESET()
#define MODBUS_UART_RELEASE_RESET()   __HAL_RCC_USART1_RELEASE_RESET()
#define MODBUS_UART_IRQn         USART1_IRQn
#define MODBUS_UART_IRQHandler   USART1_IRQHandler

#define MODBUS_UART_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define MODBUS_UART_GPIO_PORT GPIOA
#define MODBUS_UART_GPIO_AF   GPIO_AF7_USART1
#define MODBUS_UART_TX_PIN    GPIO_PIN_9
#define MODBUS_UART_RX_PIN    GPIO_PIN_10
#define MODBUS_UART_DE_PIN    GPIO_PIN_12

#define MODBUS_DMA_Instance      DMA2
#define MODBUS_DMA_CLK_ENABLE()  __HAL_RCC_DMA2_CLK_ENABLE()
#define MODBUS_DMA_TX_STREAM     DMA2_Stream7
#define MODBUS_DMA_TX_CHANNEL    DMA_CHANNEL_4
#define MODBUS_DMA_TX_IRQn       DMA2_Stream7_IRQn
#define MODBUS_DMA_TX_IRQHandler DMA2_Stream7_IRQHandler
#define MODBUS_DMA_RX_STREAM     DMA2_Stream2
#define MODBUS_DMA_RX_CHANNEL    DMA_CHANNEL_4
#define MODBUS_DMA_RX_IRQn       DMA2_Stream2_IRQn
#define MODBUS_DMA_RX_IRQHandler DMA2_Stream2_IRQHandler

void MODBUS_UART_Init();
