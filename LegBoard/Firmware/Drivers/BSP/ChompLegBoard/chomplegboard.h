#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "stm32f7xx_hal.h"

/* Definition for DAC_SPI clock resources */
#define DAC_SPI_Instance                 SPI1
#define DAC_SPI_CLK_ENABLE()             __HAL_RCC_SPI1_CLK_ENABLE()
#define DAC_SPI_SCK_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
#define DAC_SPI_MISO_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define DAC_SPI_MOSI_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define DAC_SPI_NSS_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()

#define DAC_SPI_FORCE_RESET()            __HAL_RCC_SPI1_FORCE_RESET()
#define DAC_SPI_RELEASE_RESET()          __HAL_RCC_SPI1_RELEASE_RESET()

/* Definition for DAC_SPI Pins */
#define DAC_SPI_SCK_PIN                  GPIO_PIN_5
#define DAC_SPI_SCK_GPIO_PORT            GPIOA
#define DAC_SPI_SCK_AF                   GPIO_AF5_SPI1
#define DAC_SPI_MISO_PIN                 GPIO_PIN_6
#define DAC_SPI_MISO_GPIO_PORT           GPIOA
#define DAC_SPI_MISO_AF                  GPIO_AF5_SPI1
#define DAC_SPI_MOSI_PIN                 GPIO_PIN_7
#define DAC_SPI_MOSI_GPIO_PORT           GPIOA
#define DAC_SPI_MOSI_AF                  GPIO_AF5_SPI1
#define DAC_SPI_NSS_PIN                  GPIO_PIN_4
#define DAC_SPI_NSS_GPIO_PORT            GPIOA
#define DAC_SPI_NSS_AF                   GPIO_AF5_SPI1
/* Definition for DAC LDAC/CLR Pins */
#define DAC_LDAC_PIN                     GPIO_PIN_9
#define DAC_LDAC_GPIO_PORT               GPIOC
#define DAC_CLR_PIN                      GPIO_PIN_8
#define DAC_CLR_GPIO_PORT                GPIOC

/* LDAC and CLR pulse duration in ns */
#define DAC_PULSE_DURATION               50


/* Definition for DAC_SPI's NVIC */
#define DAC_SPI_IRQn                     SPI1_IRQn
#define DAC_SPI_IRQHandler               SPI1_IRQHandler

int DAC_IO_Init(void);
void DAC_IO_Write(uint8_t write[3]);
void DAC_IO_ReadWrite(uint8_t write[3], uint8_t read[3]);
void DAC_IO_WaitForTransfer(void);
void DAC_IO_LDAC_pulse();
void DAC_IO_CLR(bool state);
void DAC_IO_CLR_pulse();
void DAC_IO_TransferComplete(SPI_HandleTypeDef *hspi);
void DAC_IO_TransferError(SPI_HandleTypeDef *hspi);


#define LED_I2C_CLK_PERIPH         RCC_PERIPHCLK_I2C1
#define LED_I2C_CLK_SRC            RCC_I2C1CLKSOURCE_PCLK1;
#define LED_I2C_Instance           I2C1
#define LED_I2C_CLK_ENABLE()       __HAL_RCC_I2C1_CLK_ENABLE()
#define LED_I2C_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()

#define LED_I2C_FORCE_RESET()      __HAL_RCC_I2C1_FORCE_RESET()
#define LED_I2C_RELEASE_RESET()    __HAL_RCC_I2C1_RELEASE_RESET()

#define LED_I2C_EV_IRQn       I2C1_EV_IRQn
#define LED_I2C_ER_IRQn       I2C1_ER_IRQn
#define LED_I2C_EV_IRQHandler I2C1_EV_IRQHandler
#define LED_I2C_ER_IRQHandler I2C1_ER_IRQHandler

#define LED_I2C_SCL_AF    GPIO_AF4_I2C1
#define LED_I2C_SCL_PIN   GPIO_PIN_6
#define LED_I2C_SDA_AF    GPIO_AF4_I2C1
#define LED_I2C_SDA_PIN   GPIO_PIN_7
#define LED_I2C_GPIO_PORT GPIOB
#define LED_EN_PIN        GPIO_PIN_8
#define LED_EN_GPIO_PORT  GPIOB

void LED_IO_Complete(void);
