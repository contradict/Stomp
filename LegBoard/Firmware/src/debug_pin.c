#include "stm32f7xx_hal.h"
#include "debug_pin.h"

#define DEBUG_PIN_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define DEBUG_PIN_PORT              GPIOB
#define DEBUG_PIN0                  GPIO_PIN_12

#define UPSTREAM_PIN     GPIO_PIN_1
#define DOWNSTREAM_PIN   GPIO_PIN_2

#define DEBUG_UPSTREAM

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

#ifdef DEBUG_UPSTREAM
    GPIO_InitStruct.Pin = UPSTREAM_PIN;
    HAL_GPIO_Init(DEBUG_PIN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DEBUG_PIN_PORT, UPSTREAM_PIN, GPIO_PIN_RESET);
#endif
#ifdef DEBUG_DOWNSTREAM
    GPIO_InitStruct.Pin = DOWNSTREAM_PIN;
    HAL_GPIO_Init(DEBUG_PIN_PORT, &GPIO_InitStruct);
    HAL_GPIO_WritePin(DEBUG_PIN_PORT, DOWNSTREAM_PIN, GPIO_PIN_RESET);
#endif
}

void DebugPin_Set(int pin, bool state)
{
    switch(pin)
    {
        case 0:
            HAL_GPIO_WritePin(DEBUG_PIN_PORT, DEBUG_PIN0, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
#ifdef DEBUG_UPSTREAM
        case 1:
            HAL_GPIO_WritePin(DEBUG_PIN_PORT, UPSTREAM_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
#endif
#ifdef DEBUG_DOWNSTREAM
        case 2:
            HAL_GPIO_WritePin(DEBUG_PIN_PORT, DOWNSTREAM_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            break;
#endif
    }
}

