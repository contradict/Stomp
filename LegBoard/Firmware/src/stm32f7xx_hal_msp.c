/**
  ******************************************************************************
  * @file    stm32f7xx_hal_msp_template.c
  * @author  MCD Application Team
  * @brief   HAL MSP module.
  *          This file template is located in the HAL folder and should be copied 
  *          to the user folder.
  *         
  @verbatim
 ===============================================================================
                     ##### How to use this driver #####
 ===============================================================================
    [..]

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "main.h"
#include "feedback_adc.h"
#include "modbus_uart.h"
#include "chomplegboard.h"
#include "servo_uart.h"

/** @addtogroup STM32F7xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP HAL MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions HAL MSP Private Functions
  * @{
  */

/**
  * @brief  Initializes the Global MSP.
  * @retval None
  */
void HAL_MspInit(void)
{
 
}

/**
  * @brief  DeInitializes the Global MSP.  
  * @retval None
  */
void HAL_MspDeInit(void)
{

}

/**
  * @brief  Initializes the PPP MSP.
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    if(hspi->Instance == DAC_SPI_Instance)
    {
        DAC_SPI_SCK_GPIO_CLK_ENABLE();
        DAC_SPI_MISO_GPIO_CLK_ENABLE();
        DAC_SPI_MOSI_GPIO_CLK_ENABLE();
        DAC_SPI_NSS_GPIO_CLK_ENABLE();

        DAC_SPI_CLK_ENABLE();

        /* SCK */
        GPIO_InitStruct.Pin = DAC_SPI_SCK_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = DAC_SPI_SCK_AF;
        HAL_GPIO_Init(DAC_SPI_SCK_GPIO_PORT, &GPIO_InitStruct);
        /* MISO */
        GPIO_InitStruct.Pin = DAC_SPI_MISO_PIN;
        GPIO_InitStruct.Alternate = DAC_SPI_MISO_AF;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(DAC_SPI_MISO_GPIO_PORT, &GPIO_InitStruct);
        /* MOSI */
        GPIO_InitStruct.Pin = DAC_SPI_MOSI_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Alternate = DAC_SPI_MOSI_AF;
        HAL_GPIO_Init(DAC_SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);
        /* NSS */
        GPIO_InitStruct.Pin = DAC_SPI_NSS_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Alternate = 0;
        HAL_GPIO_Init(DAC_SPI_NSS_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(DAC_SPI_IRQn, 6, 0);
        HAL_NVIC_EnableIRQ(DAC_SPI_IRQn);
     }
}

/**
  * @brief  DeInitializes the PPP MSP.  
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
    if(hspi->Instance == DAC_SPI_Instance)
    {
        DAC_SPI_FORCE_RESET();
        DAC_SPI_RELEASE_RESET();
        HAL_GPIO_DeInit(DAC_SPI_SCK_GPIO_PORT, DAC_SPI_SCK_PIN);
        HAL_GPIO_DeInit(DAC_SPI_MISO_GPIO_PORT, DAC_SPI_MISO_PIN);
        HAL_GPIO_DeInit(DAC_SPI_MOSI_GPIO_PORT, DAC_SPI_MOSI_PIN);
        HAL_GPIO_DeInit(DAC_SPI_NSS_GPIO_PORT, DAC_SPI_NSS_PIN);
        HAL_NVIC_DisableIRQ(DAC_SPI_IRQn);
    }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    GPIO_InitTypeDef GPIO_InitStruct;

    FEEDBACK_ADC_CHANNEL_GPIO_CLOCK_ENABLE();
    FEEDBACK_ADC_CURL_CLK_ENABLE();
    FEEDBACK_ADC_SWING_CLK_ENABLE();
    FEEDBACK_ADC_LIFT_CLK_ENABLE();
    FEEDBACK_DMA_CLK_ENABLE();

    GPIO_InitStruct.Pin = CURL_ADC_CHANNEL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FEEDBACK_ADC_CHANNEL_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = SWING_ADC_CHANNEL_PIN;
    HAL_GPIO_Init(FEEDBACK_ADC_CHANNEL_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LIFT_ADC_CHANNEL_PIN;
    HAL_GPIO_Init(FEEDBACK_ADC_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

    static DMA_HandleTypeDef hdma_adc;
    hdma_adc.Instance = FEEDBACK_ADC_DMA_STREAM;
    hdma_adc.Init.Channel = FEEDBACK_ADC_DMA_CHANNEL;
    hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_adc.Init.Mode = DMA_CIRCULAR;
    hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
    hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_adc.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_HALFFULL;
    hdma_adc.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_adc.Init.PeriphBurst = DMA_PBURST_SINGLE;

    HAL_DMA_Init(&hdma_adc);

    __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

    HAL_NVIC_SetPriority(FEEDBACK_DMA_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(FEEDBACK_DMA_IRQn);
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    FEEDBACK_ADC_FORCE_RESET();
    FEEDBACK_ADC_RELEASE_RESET();
    HAL_GPIO_DeInit(FEEDBACK_ADC_CHANNEL_GPIO_PORT, CURL_ADC_CHANNEL_PIN);
    HAL_GPIO_DeInit(FEEDBACK_ADC_CHANNEL_GPIO_PORT, SWING_ADC_CHANNEL_PIN);
    HAL_GPIO_DeInit(FEEDBACK_ADC_CHANNEL_GPIO_PORT, LIFT_ADC_CHANNEL_PIN);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim == &feedback_timer)
    {
        FEEDBACK_TRIGGER_TIM_CLK_ENABLE();
    }
}

void HAL_TIM_Base_MspDepInit(TIM_HandleTypeDef *htim)
{
    if(htim == &feedback_timer)
    {
        FEEDBACK_TRIGGER_TIM_FORCE_RESET();
        FEEDBACK_TRIGGER_TIM_RELEASE_RESET();
    }
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct;
    if(hi2c->Instance == LED_I2C_Instance)
    {

        /*##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #*/
        RCC_PeriphCLKInitStruct.PeriphClockSelection = LED_I2C_CLK_PERIPH;
        RCC_PeriphCLKInitStruct.I2c1ClockSelection = LED_I2C_CLK_SRC;
        HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

        LED_I2C_GPIO_CLK_ENABLE();
        LED_I2C_CLK_ENABLE();

        /* SCL */
        GPIO_InitStruct.Pin = LED_I2C_SCL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = LED_I2C_SCL_AF;
        HAL_GPIO_Init(LED_I2C_GPIO_PORT, &GPIO_InitStruct);
        /* SDA */
        GPIO_InitStruct.Pin = LED_I2C_SDA_PIN;
        GPIO_InitStruct.Alternate = LED_I2C_SDA_AF;
        HAL_GPIO_Init(LED_I2C_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(LED_I2C_EV_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(LED_I2C_EV_IRQn);
        HAL_NVIC_SetPriority(LED_I2C_ER_IRQn, 7, 0);
        HAL_NVIC_EnableIRQ(LED_I2C_ER_IRQn);
    }
}

void HAL_UART_MspInit_modbus(UART_HandleTypeDef *huart)
{
    (void)huart;
    GPIO_InitTypeDef GPIO_InitStruct;

    MODBUS_UART_GPIO_CLK_ENABLE();

    MODBUS_UART_CLK_ENABLE();

    GPIO_InitStruct.Pin = MODBUS_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = MODBUS_UART_GPIO_AF;
    HAL_GPIO_Init(MODBUS_UART_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = MODBUS_UART_RX_PIN;
    HAL_GPIO_Init(MODBUS_UART_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = MODBUS_UART_DE_PIN;
    HAL_GPIO_Init(MODBUS_UART_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(MODBUS_UART_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(MODBUS_UART_IRQn);
}

void HAL_UART_MspInit_Curl(UART_HandleTypeDef *huart)
{
    static DMA_HandleTypeDef tx_dma;
    static DMA_HandleTypeDef rx_dma;

    GPIO_InitTypeDef GPIO_InitStruct;

    CURL_UART_GPIO_CLK_ENABLE();

    CURL_UART_CLK_ENABLE();

    CURL_DMA_CLK_ENABLE();

    GPIO_InitStruct.Pin = MODBUS_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = MODBUS_UART_GPIO_AF;
    HAL_GPIO_Init(MODBUS_UART_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = MODBUS_UART_RX_PIN;
    HAL_GPIO_Init(MODBUS_UART_GPIO_PORT, &GPIO_InitStruct);

    tx_dma.Instance                 = CURL_DMA_TX_STREAM;
    tx_dma.Init.Channel             = CURL_DMA_TX_CHANNEL;
    tx_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    tx_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    tx_dma.Init.MemInc              = DMA_MINC_ENABLE;
    tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    tx_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    tx_dma.Init.Mode                = DMA_NORMAL;
    tx_dma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&tx_dma);
    __HAL_LINKDMA(huart, hdmatx, tx_dma);

    rx_dma.Instance                 = CURL_DMA_RX_STREAM;
    rx_dma.Init.Channel             = CURL_DMA_RX_CHANNEL;
    rx_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    rx_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    rx_dma.Init.MemInc              = DMA_MINC_ENABLE;
    rx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    rx_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    rx_dma.Init.Mode                = DMA_NORMAL;
    rx_dma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&rx_dma);
    __HAL_LINKDMA(huart, hdmarx, rx_dma);

    HAL_NVIC_SetPriority(CURL_DMA_TX_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(CURL_DMA_TX_IRQn);

    HAL_NVIC_SetPriority(CURL_DMA_RX_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(CURL_DMA_RX_IRQn);

    HAL_NVIC_SetPriority(CURL_UART_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(CURL_UART_IRQn);
}

void HAL_UART_MspInit_Lift(UART_HandleTypeDef *huart)
{
    static DMA_HandleTypeDef tx_dma;
    static DMA_HandleTypeDef rx_dma;

    GPIO_InitTypeDef GPIO_InitStruct;

    LIFT_UART_GPIO_CLK_ENABLE();

    LIFT_UART_CLK_ENABLE();

    LIFT_DMA_CLK_ENABLE();

    GPIO_InitStruct.Pin = LIFT_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = LIFT_UART_GPIO_AF;
    HAL_GPIO_Init(LIFT_UART_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LIFT_UART_RX_PIN;
    HAL_GPIO_Init(LIFT_UART_GPIO_PORT, &GPIO_InitStruct);

    tx_dma.Instance                 = LIFT_DMA_TX_STREAM;
    tx_dma.Init.Channel             = LIFT_DMA_TX_CHANNEL;
    tx_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    tx_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    tx_dma.Init.MemInc              = DMA_MINC_ENABLE;
    tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    tx_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    tx_dma.Init.Mode                = DMA_NORMAL;
    tx_dma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&tx_dma);
    __HAL_LINKDMA(huart, hdmatx, tx_dma);

    rx_dma.Instance                 = LIFT_DMA_RX_STREAM;
    rx_dma.Init.Channel             = LIFT_DMA_RX_CHANNEL;
    rx_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    rx_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    rx_dma.Init.MemInc              = DMA_MINC_ENABLE;
    rx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    rx_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    rx_dma.Init.Mode                = DMA_NORMAL;
    rx_dma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&rx_dma);
    __HAL_LINKDMA(huart, hdmarx, rx_dma);

    HAL_NVIC_SetPriority(LIFT_DMA_TX_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(LIFT_DMA_TX_IRQn);

    HAL_NVIC_SetPriority(LIFT_DMA_RX_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(LIFT_DMA_RX_IRQn);

    HAL_NVIC_SetPriority(LIFT_UART_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(LIFT_UART_IRQn);
}

void HAL_UART_MspInit_Swing(UART_HandleTypeDef *huart)
{
    static DMA_HandleTypeDef tx_dma;
    static DMA_HandleTypeDef rx_dma;

    GPIO_InitTypeDef GPIO_InitStruct;

    SWING_UART_GPIO_CLK_ENABLE();

    SWING_UART_CLK_ENABLE();

    SWING_DMA_CLK_ENABLE();

    GPIO_InitStruct.Pin = SWING_UART_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = SWING_UART_GPIO_AF;
    HAL_GPIO_Init(SWING_UART_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = SWING_UART_RX_PIN;
    HAL_GPIO_Init(SWING_UART_GPIO_PORT, &GPIO_InitStruct);

    tx_dma.Instance                 = SWING_DMA_TX_STREAM;
    tx_dma.Init.Channel             = SWING_DMA_TX_CHANNEL;
    tx_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    tx_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    tx_dma.Init.MemInc              = DMA_MINC_ENABLE;
    tx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    tx_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    tx_dma.Init.Mode                = DMA_NORMAL;
    tx_dma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&tx_dma);
    __HAL_LINKDMA(huart, hdmatx, tx_dma);

    rx_dma.Instance                 = SWING_DMA_RX_STREAM;
    rx_dma.Init.Channel             = SWING_DMA_RX_CHANNEL;
    rx_dma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    rx_dma.Init.PeriphInc           = DMA_PINC_DISABLE;
    rx_dma.Init.MemInc              = DMA_MINC_ENABLE;
    rx_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    rx_dma.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    rx_dma.Init.Mode                = DMA_NORMAL;
    rx_dma.Init.Priority            = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&rx_dma);
    __HAL_LINKDMA(huart, hdmarx, rx_dma);

    HAL_NVIC_SetPriority(SWING_DMA_TX_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(SWING_DMA_TX_IRQn);

    HAL_NVIC_SetPriority(SWING_DMA_RX_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(SWING_DMA_RX_IRQn);

    HAL_NVIC_SetPriority(SWING_UART_IRQn, 6, 1);
    HAL_NVIC_EnableIRQ(SWING_UART_IRQn);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if(huart == &modbus_uart)
    {
        HAL_UART_MspInit_modbus(huart);
    } else if(huart == &servo_uart[CURL])
    {
        HAL_UART_MspInit_Curl(huart);
    } else if(huart == &servo_uart[LIFT])
    {
        HAL_UART_MspInit_Lift(huart);
    } else if(huart == &servo_uart[SWING])
    {
        HAL_UART_MspInit_Swing(huart);
    }
}

void HAL_CRC_MspInit(CRC_HandleTypeDef *hcrc)
{
    (void)hcrc;
    __HAL_RCC_CRC_CLK_ENABLE();
}

void HAL_CRC_MspDeInit(CRC_HandleTypeDef *hcrc)
{
    (void)hcrc;
    __HAL_RCC_CRC_FORCE_RESET();
    __HAL_RCC_CRC_RELEASE_RESET();
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
