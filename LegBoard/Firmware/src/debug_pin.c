#include "stm32f7xx_hal.h"
#include "debug_pin.h"

#define DEBUG_PIN_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define DEBUG_PIN_PORT              GPIOB
#define DEBUG_PIN0                  GPIO_PIN_12

void DebugPin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    DEBUG_PIN_GPIO_CLK_ENABLE();
    GPIO_InitStruct.Pin = DEBUG_PIN0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(DEBUG_PIN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DEBUG_PIN_PORT, DEBUG_PIN0, GPIO_PIN_RESET);
}

void DebugPin_Set(int pin, bool state)
{
    switch(pin)
    {
        case 0:
            HAL_GPIO_WritePin(DEBUG_PIN_PORT, DEBUG_PIN0, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
    }
}

