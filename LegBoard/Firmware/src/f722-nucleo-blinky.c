#include <stdbool.h>
#include "cmsis_os.h"
#include "stm32f7xx_nucleo_144.h"

#define UU(x) __attribute__((unused)) x

osThreadId buttonThreadId;

void led1_thread(UU(void const *args)) {
    while (true) {
        BSP_LED_Toggle(LED_RED);
        osDelay(1000);
    }
}

void led2_thread(UU(void const *args)) {
    while (true) {
        BSP_LED_Toggle(LED_GREEN);
        osDelay(500);
    }
}

void led3_thread(UU(void const *args)) {
    while (true) {
        BSP_LED_Toggle(LED_BLUE);
        osDelay(250);
    }
}


void HAL_GPIO_EXTI_Callback(UU(uint16_t GPIO_Pin))
{
  osSignalSet(buttonThreadId, 0);
}

void button_thread(UU(void const *arg))
{
    osEvent event;
    for(;;)
    {
        event = osSignalWait(0, osWaitForever);
        osDelay(10);
        if(BSP_PB_GetState(BUTTON_USER) == 1)
        {
            BSP_LED_Toggle(LED_RED);
            BSP_LED_Toggle(LED_BLUE);
            BSP_LED_Toggle(LED_GREEN);
        }
    }
}

void f722_nucleo_blinky_Init(void)
{
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_RED);
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  osThreadDef(led1, led1_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(led2, led2_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(led3, led3_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadDef(button, button_thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

  osThreadCreate(osThread(led1), NULL);
  osThreadCreate(osThread(led2), NULL);
  osThreadCreate(osThread(led3), NULL);
  buttonThreadId = osThreadCreate(osThread(button), NULL);
}


