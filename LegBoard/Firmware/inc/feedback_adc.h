#include "stm32f7xx_hal.h"

extern TIM_HandleTypeDef feedback_timer;
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Definition for ADCx clock resources */
#define FEEDBACK_ADC                              ADC1
#define FEEDBACK_ADC_CLK_ENABLE()                 __HAL_RCC_ADC1_CLK_ENABLE()
#define FEEDBACK_ADC_CHANNEL_GPIO_CLOCK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()

#define FEEDBACK_ADC_FORCE_RESET()        __HAL_RCC_ADC_FORCE_RESET()
#define FEEDBACK_ADC_RELEASE_RESET()      __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADC Channel Pin */
#define FEEDBACK_ADC_CHANNEL_GPIO_PORT    GPIOC
#define CURL_ADC_CHANNEL_PIN              GPIO_PIN_1
#define SWING_ADC_CHANNEL_PIN             GPIO_PIN_2
#define LIFT_ADC_CHANNEL_PIN              GPIO_PIN_3

/* Definition for ADC's Channel */
#define CURL_ADC_CHANNEL                  ADC_CHANNEL_11
#define SWING_ADC_CHANNEL                 ADC_CHANNEL_12
#define LIFT_ADC_CHANNEL                  ADC_CHANNEL_13

/* Definition for SWING_ADC's NVIC */
#define FEEDBACK_ADC_IRQn                 ADC_IRQn
#define FEEDBACK_ADC_IRQHandler           ADC_IRQHandler

/* Definition for TIMx clock resources */
#define FEEDBACK_TRIGGER_TIM                 TIM2
#define FEEDBACK_TRIGGER_TIM_CLK_ENABLE()    __HAL_RCC_TIM2_CLK_ENABLE()

#define FEEDBACK_TRIGGER_TIM_FORCE_RESET()   __HAL_RCC_TIM2_FORCE_RESET()
#define FEEDBACK_TRIGGER_TIM_RELEASE_RESET() __HAL_RCC_TIM2_RELEASE_RESET()

void FeedbackADC_Init(void);
void FeedbackADC_TimerInit(void);
