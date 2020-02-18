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
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = DAC_SPI_SCK_AF;
        HAL_GPIO_Init(DAC_SPI_SCK_GPIO_PORT, &GPIO_InitStruct);
        /* MISO */
        GPIO_InitStruct.Pin = DAC_SPI_MISO_PIN;
        GPIO_InitStruct.Alternate = DAC_SPI_MISO_AF;
        HAL_GPIO_Init(DAC_SPI_MISO_GPIO_PORT, &GPIO_InitStruct);
        /* MOSI */
        GPIO_InitStruct.Pin = DAC_SPI_MOSI_PIN;
        GPIO_InitStruct.Alternate = DAC_SPI_MOSI_AF;
        HAL_GPIO_Init(DAC_SPI_MOSI_GPIO_PORT, &GPIO_InitStruct);
        /* NSS */
        GPIO_InitStruct.Pin = DAC_SPI_NSS_PIN;
        GPIO_InitStruct.Alternate = DAC_SPI_NSS_AF;
        HAL_GPIO_Init(DAC_SPI_NSS_GPIO_PORT, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(DAC_SPI_IRQn, 1, 0);
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
    GPIO_InitTypeDef GPIO_InitStruct;

    FEEDBACK_ADC_CHANNEL_GPIO_CLOCK_ENABLE();
    FEEDBACK_ADC_CLK_ENABLE();
    GPIO_InitStruct.Pin = CURL_ADC_CHANNEL_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(FEEDBACK_ADC_CHANNEL_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = SWING_ADC_CHANNEL_PIN;
    HAL_GPIO_Init(FEEDBACK_ADC_CHANNEL_GPIO_PORT, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = LIFT_ADC_CHANNEL_PIN;
    HAL_GPIO_Init(FEEDBACK_ADC_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(FEEDBACK_ADC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FEEDBACK_ADC_IRQn);
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
{
    FEEDBACK_ADC_FORCE_RESET();
    FEEDBACK_ADC_RELEASE_RESET();
    HAL_GPIO_DeInit(FEEDBACK_ADC_CHANNEL_GPIO_PORT, CURL_ADC_CHANNEL_PIN);
    HAL_GPIO_DeInit(FEEDBACK_ADC_CHANNEL_GPIO_PORT, SWING_ADC_CHANNEL_PIN);
    HAL_GPIO_DeInit(FEEDBACK_ADC_CHANNEL_GPIO_PORT, LIFT_ADC_CHANNEL_PIN);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    FEEDBACK_TRIGGER_TIM_CLK_ENABLE();
}

void HAL_TIM_Base_MsaDepInit(TIM_HandleTypeDef *htim)
{
    FEEDBACK_TRIGGER_TIM_FORCE_RESET();
    FEEDBACK_TRIGGER_TIM_RELEASE_RESET();
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
