#include <math.h>
#include "chomplegboard.h"
#include "stm32f7xx_hal.h"

extern SPI_HandleTypeDef DAC_SPIHandle;

volatile static bool transfer_started;
static int dwt_started = 0;

void enableCycleCounter(void)
{
#if (__CORTEX_M >= 0x03U)
    if(!dwt_started)
    {
        dwt_started = 1;
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#if __CORTEX_M == 7
        // Unlock DWT.
        DWT->LAR = 0xC5ACCE55;
#endif
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        DWT->CYCCNT = 0;
    }
#endif
}

void disableCycleCounter(void)
{
#if (__CORTEX_M >= 0x03U)
    if(dwt_started)
    {
        dwt_started = 0;
#if __CORTEX_M == 7
        // Unlock DWT.
        DWT->LAR = 0xC5ACCE55;
#endif
        DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk;
        CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk;
    }
#endif
}

uint32_t getCycleCount ()
{
#if (__CORTEX_M >= 0x03U)
    return DWT->CYCCNT;
#else
    return 0;
#endif
}

void delayCycles (uint32_t const numCycles)
{
#if (__CORTEX_M >= 0x03U)
  uint32_t const startCycles = getCycleCount();
  while ((getCycleCount() - startCycles) < numCycles) { }
#endif
}

uint32_t nanosecondsToCycles (uint32_t const nanoseconds) {
  return ceilf(nanoseconds * ((float) SystemCoreClock / 1e9f));
}

int DAC_IO_Init(void)
{
    enableCycleCounter();

    GPIO_InitTypeDef GPIO_InitStruct;

    /* LDAC */
    GPIO_InitStruct.Pin = DAC_LDAC_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(DAC_LDAC_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DAC_CLR_GPIO_PORT, DAC_CLR_PIN, GPIO_PIN_SET);
    /* CLR */
    GPIO_InitStruct.Pin = DAC_CLR_PIN;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(DAC_CLR_GPIO_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DAC_CLR_GPIO_PORT, DAC_CLR_PIN, GPIO_PIN_SET);

    DAC_SPIHandle.Instance = DAC_SPI_Instance;
    DAC_SPIHandle.Init.CLKPhase = SPI_DIRECTION_2LINES;
    DAC_SPIHandle.Init.Mode = SPI_MODE_MASTER;
    DAC_SPIHandle.Init.Direction = SPI_DIRECTION_2LINES;
    DAC_SPIHandle.Init.DataSize = SPI_DATASIZE_8BIT;
    DAC_SPIHandle.Init.CLKPolarity = SPI_POLARITY_LOW;
    DAC_SPIHandle.Init.CLKPhase = SPI_PHASE_2EDGE;
    DAC_SPIHandle.Init.NSS = SPI_NSS_HARD_OUTPUT;
    DAC_SPIHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    DAC_SPIHandle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    DAC_SPIHandle.Init.TIMode = SPI_TIMODE_DISABLE;
    DAC_SPIHandle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    DAC_SPIHandle.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;

    return HAL_SPI_Init(&DAC_SPIHandle);
}

void DAC_IO_Write(uint32_t* write)
{
    transfer_started = 1;
    HAL_SPI_Transmit_IT(&DAC_SPIHandle, ((uint8_t *)write), 3); 
}

void DAC_IO_ReadWrite(uint32_t *write, uint32_t* read)
{
    transfer_started = 1;
    HAL_SPI_TransmitReceive_IT(&DAC_SPIHandle,
                               ((uint8_t *)write),
                               ((uint8_t *)read), 3); 
}

void DAC_IO_WaitForTransfer(void)
{
    while(transfer_started);
}

void DAC_IO_LDAC(bool state)
{
  HAL_GPIO_WritePin(DAC_LDAC_GPIO_PORT, DAC_LDAC_PIN, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void DAC_IO_LDAC_pulse(void)
{
  bool state = HAL_GPIO_ReadPin(DAC_LDAC_GPIO_PORT, DAC_LDAC_PIN);
  HAL_GPIO_WritePin(DAC_LDAC_GPIO_PORT, DAC_LDAC_PIN, !state ? GPIO_PIN_RESET : GPIO_PIN_SET);
  delayCycles(nanosecondsToCycles(DAC_PULSE_DURATION));
  HAL_GPIO_WritePin(DAC_LDAC_GPIO_PORT, DAC_LDAC_PIN, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void DAC_IO_CLR(bool state)
{
  HAL_GPIO_WritePin(DAC_CLR_GPIO_PORT, DAC_CLR_PIN, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void DAC_IO_CLR_pulse(void)
{
  bool state = HAL_GPIO_ReadPin(DAC_CLR_GPIO_PORT, DAC_CLR_PIN);
  HAL_GPIO_WritePin(DAC_CLR_GPIO_PORT, DAC_CLR_PIN, !state ? GPIO_PIN_RESET : GPIO_PIN_SET);
  delayCycles(nanosecondsToCycles(DAC_PULSE_DURATION));
  HAL_GPIO_WritePin(DAC_CLR_GPIO_PORT, DAC_CLR_PIN, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    transfer_started = 0;
    DAC_IO_TransferComplete(hspi);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef* hspi)
{
    DAC_IO_TransferError(hspi);
}

__weak void DAC_IO_TransferComplete(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
}

__weak void DAC_IO_TransferError(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
}
