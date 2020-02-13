/**
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "stm32f7xx_nucleo_144.h"
#include <stdbool.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DEFAULT_STACK_SIZE 0

#define UU(x) __attribute__((unused)) x
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);

/* Private functions ---------------------------------------------------------*/

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

osThreadId buttonThreadId;

void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(USER_BUTTON_PIN);
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


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* This project template calls firstly CPU_CACHE_Enable() in order to enable the CPU Cache.
     This function is provided as template implementation that User may integrate 
     in his application, to enhance the performance in case of use of AXI interface 
     with several masters. */

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32F7xx HAL library initialization:
       - Configure the Flash ART accelerator on ITCM interface
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 216 MHz */
  SystemClock_Config();

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

  /* Start scheduler */
  osKernelStart();

  /* Infinite loop */
  for(;;);
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source
  Note: Since there is no oscillator on board, HSE clock is derived from STLink */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while(1) {};
  }

  /* Activate the OverDrive to reach the 216 Mhz Frequency */
  if(HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    while(1) {};
  }


  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    while(1) {};
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
