/**
  ******************************************************************************
  * @file    Templates/Src/stm32f7xx.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
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
#include "main.h"
#include "stm32f7xx_it.h"
#include "modbus_uart.h"
#include "chomplegboard.h"
#include "feedback_adc.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern SPI_HandleTypeDef DAC_SPIHandle;
extern ADC_HandleTypeDef feedback_adc;
extern I2C_HandleTypeDef LED_I2CHandle;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
/*
void SVC_Handler(void)
{
}
*/

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/*
void PendSV_Handler(void)
{
}
*/

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/*
void SysTick_Handler(void)
{
  HAL_IncTick();
}
*/

/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
#ifdef STM32F7xx_Nucleo_144
  HAL_GPIO_EXTI_IRQHandler(USER_BUTTON_PIN);
#endif
}


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
void DAC_SPI_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&DAC_SPIHandle);
}

void FEEDBACK_ADC_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&feedback_adc);
}

/**
  * @brief  This function handles I2C event interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C data transmission
  */
void LED_I2C_EV_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&LED_I2CHandle);
}

/**
  * @brief  This function handles I2C error interrupt request.
  * @param  None
  * @retval None
  * @Note   This function is redefined in "main.h" and related to I2C error
  */
void LED_I2C_ER_IRQHandler(void)
{
  HAL_I2C_ER_IRQHandler(&LED_I2CHandle);
}

void MODBUS_UART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&modbus_uart);
}

void CURL_UART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&modbus_uart);
}

void CURL_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(modbus_uart.hdmatx);
}

void CURL_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(modbus_uart.hdmarx);
}

void LIFT_UART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&modbus_uart);
}

void LIFT_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(modbus_uart.hdmatx);
}

void LIFT_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(modbus_uart.hdmarx);
}

void SWING_UART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&modbus_uart);
}

void SWING_DMA_TX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(modbus_uart.hdmatx);
}

void SWING_DMA_RX_IRQHandler(void)
{
    HAL_DMA_IRQHandler(modbus_uart.hdmarx);
}


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
