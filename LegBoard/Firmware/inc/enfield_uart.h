#pragma once

#include "stm32f7xx_hal.h"

extern UART_HandleTypeDef enfield_uart[3];

#define CURL_UART_Instance        USART3
#define CURL_UART_CLK_ENABLE()    __HAL_RCC_USART3_CLK_ENABLE()
#define CURL_UART_FORCE_RESET()   __HAL_RCC_USART3_FORCE_RESET()
#define CURL_UART_RELEASE_RESET() __HAL_RCC_USART3_RELEASE_RESET()
#define CURL_UART_IRQn            USART3_IRQn
#define CURL_UART_IRQHandler      USART3_IRQHandler

#define CURL_UART_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define CURL_UART_GPIO_PORT         GPIOB
#define CURL_UART_GPIO_AF           GPIO_AF7_USART3
#define CURL_UART_TX_PIN            10
#define CURL_UART_RX_PIN            11

#define CURL_DMA_Instance      DMA1
#define CURL_DMA_CLK_ENABLE()  __HAL_RCC_DMA1_CLK_ENABLE()
#define CURL_DMA_TX_STREAM     DMA1_Stream3
#define CURL_DMA_TX_CHANNEL    DMA_CHANNEL_4
#define CURL_DMA_TX_IRQn       DMA1_Stream3_IRQn
#define CURL_DMA_TX_IRQHandler DMA1_Stream3_IRQHandler
#define CURL_DMA_RX_STREAM     DMA1_Stream1
#define CURL_DMA_RX_CHANNEL    DMA_CHANNEL_4
#define CURL_DMA_RX_IRQn       DMA1_Stream1_IRQn
#define CURL_DMA_RX_IRQHandler DMA1_Stream1_IRQHandler

#define LIFT_UART_Instance        USART2
#define LIFT_UART_CLK_ENABLE()    __HAL_RCC_USART2_CLK_ENABLE()
#define LIFT_UART_FORCE_RESET()   __HAL_RCC_USART2_FORCE_RESET()
#define LIFT_UART_RELEASE_RESET() __HAL_RCC_USART2_RELEASE_RESET()
#define LIFT_UART_IRQn            USART2_IRQn
#define LIFT_UART_IRQHandler      USART2_IRQHandler

#define LIFT_UART_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define LIFT_UART_GPIO_PORT         GPIOA
#define LIFT_UART_GPIO_AF           GPIO_AF7_USART2
#define LIFT_UART_TX_PIN            2
#define LIFT_UART_RX_PIN            3

#define LIFT_DMA_Instance      DMA1
#define LIFT_DMA_CLK_ENABLE()  __HAL_RCC_DMA1_CLK_ENABLE()
#define LIFT_DMA_TX_STREAM     DMA1_Stream6
#define LIFT_DMA_TX_CHANNEL    DMA_CHANNEL_4
#define LIFT_DMA_TX_IRQn       DMA1_Stream6_IRQn
#define LIFT_DMA_TX_IRQHandler DMA1_Stream6_IRQHandler
#define LIFT_DMA_RX_STREAM     DMA1_Stream5
#define LIFT_DMA_RX_CHANNEL    DMA_CHANNEL_4
#define LIFT_DMA_RX_IRQn       DMA1_Stream5_IRQn
#define LIFT_DMA_RX_IRQHandler DMA1_Stream5_IRQHandler

#define SWING_UART_Instance        USART6
#define SWING_UART_CLK_ENABLE()    __HAL_RCC_USART6_CLK_ENABLE()
#define SWING_UART_FORCE_RESET()   __HAL_RCC_USART6_FORCE_RESET()
#define SWING_UART_RELEASE_RESET() __HAL_RCC_USART6_RELEASE_RESET()
#define SWING_UART_IRQn            USART6_IRQn
#define SWING_UART_IRQHandler      USART6_IRQHandler

#define SWING_UART_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define SWING_UART_GPIO_PORT         GPIOC
#define SWING_UART_GPIO_AF           GPIO_AF8_USART6
#define SWING_UART_TX_PIN            6
#define SWING_UART_RX_PIN            7

#define SWING_DMA_Instance      DMA2
#define SWING_DMA_CLK_ENABLE()  __HAL_RCC_DMA2_CLK_ENABLE()
#define SWING_DMA_TX_STREAM     DMA2_Stream6
#define SWING_DMA_TX_CHANNEL    DMA_CHANNEL_5
#define SWING_DMA_TX_IRQn       DMA2_Stream6_IRQn
#define SWING_DMA_TX_IRQHandler DMA2_Stream6_IRQHandler
#define SWING_DMA_RX_STREAM     DMA2_Stream1
#define SWING_DMA_RX_CHANNEL    DMA_CHANNEL_5
#define SWING_DMA_RX_IRQn       DMA2_Stream1_IRQn
#define SWING_DMA_RX_IRQHandler DMA2_Stream1_IRQHandler

void Enfield_UART_Init();
