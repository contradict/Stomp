#include "stm32f7xx_hal.h"
#include "feedback_adc.h"

extern ADC_HandleTypeDef feedback_adc;
TIM_HandleTypeDef feedback_timer;

void FeedbackADC_Init(void)
{
    ADC_ChannelConfTypeDef sConfig;

    feedback_adc.Instance = FEEDBACK_ADC;

    // Clocked from PCLK2 at 108MHz, div4 gives 27MHz ADCCLK
    feedback_adc.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
    feedback_adc.Init.Resolution            = ADC_RESOLUTION_12B;
    feedback_adc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    feedback_adc.Init.NbrOfConversion       = 3;
    feedback_adc.Init.ScanConvMode          = DISABLE;
    feedback_adc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    feedback_adc.Init.ContinuousConvMode    = DISABLE;
    feedback_adc.Init.DiscontinuousConvMode = ENABLE;
    feedback_adc.Init.NbrOfDiscConversion   = 1;
    feedback_adc.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T2_TRGO;
    feedback_adc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;
    feedback_adc.Init.DMAContinuousRequests = DISABLE;

    HAL_ADC_Init(&feedback_adc);

    sConfig.Channel = CURL_ADC_CHANNEL;
    sConfig.Rank    = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&feedback_adc, &sConfig);
    sConfig.Channel = LIFT_ADC_CHANNEL;
    sConfig.Rank    = 2;
    HAL_ADC_ConfigChannel(&feedback_adc, &sConfig);
    sConfig.Channel = SWING_ADC_CHANNEL;
    sConfig.Rank    = 3;
    HAL_ADC_ConfigChannel(&feedback_adc, &sConfig);
}

void FeedbackADC_TimerInit(void)
{
    TIM_MasterConfigTypeDef mstrconfig;

    feedback_timer.Instance = FEEDBACK_TRIGGER_TIM;

    // Clocked from APB1 x 2, APB1 prescaler is 4, = 216MHz/2 time clock
    // (216MHz/2) / (3kHz) = 72000
    // Max prescaler is 65535, use prescaler
    feedback_timer.Init.Prescaler = 9; // prescale is value + 1
    feedback_timer.Init.Period = 7200;
    feedback_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    feedback_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    feedback_timer.Init.RepetitionCounter = 0x0;
    feedback_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&feedback_timer);

    mstrconfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    mstrconfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&feedback_timer, &mstrconfig);
}


