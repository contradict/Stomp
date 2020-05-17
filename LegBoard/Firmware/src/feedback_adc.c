#include "stm32f7xx_hal.h"
#include "feedback_adc.h"
#include "export/joint.h"

ADC_HandleTypeDef feedback_adcs[JOINT_COUNT];
TIM_HandleTypeDef feedback_timer;

void FeedbackADC_Init(void)
{
    ADC_ChannelConfTypeDef sConfig;

    /* Done in ADC order since 1 must be the master */
    feedback_adcs[0].Instance = ADC1;
    feedback_adcs[1].Instance = ADC2;
    feedback_adcs[2].Instance = ADC3;

    feedback_adcs[0].Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;
    feedback_adcs[1].Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
    feedback_adcs[2].Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;

    feedback_adcs[0].Init.DMAContinuousRequests = ENABLE;
    feedback_adcs[1].Init.DMAContinuousRequests = DISABLE;
    feedback_adcs[2].Init.DMAContinuousRequests = DISABLE;

    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    sConfig.Offset = 0;

    for(int adcidx=2; adcidx>=0; adcidx--)
    {
        // Clocked from PCLK2 at 108MHz, div4 gives 27MHz ADCCLK
        feedback_adcs[adcidx].Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
        feedback_adcs[adcidx].Init.Resolution            = ADC_RESOLUTION_12B;
        feedback_adcs[adcidx].Init.DataAlign             = ADC_DATAALIGN_RIGHT;
        feedback_adcs[adcidx].Init.NbrOfConversion       = 1;
        feedback_adcs[adcidx].Init.ScanConvMode          = DISABLE;
        feedback_adcs[adcidx].Init.EOCSelection          = DISABLE;
        feedback_adcs[adcidx].Init.ContinuousConvMode    = DISABLE;
        feedback_adcs[adcidx].Init.DiscontinuousConvMode = ENABLE;
        feedback_adcs[adcidx].Init.NbrOfDiscConversion   = 1;
        feedback_adcs[adcidx].Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T2_TRGO;

        HAL_ADC_Init(&feedback_adcs[adcidx]);

        sConfig.Channel = ADC_CHANNEL_11 + adcidx;

        HAL_ADC_ConfigChannel(&feedback_adcs[adcidx], &sConfig);
    }

    ADC_MultiModeTypeDef mode;
    mode.Mode = ADC_TRIPLEMODE_REGSIMULT;
    mode.DMAAccessMode = ADC_DMAACCESSMODE_1;
    mode.TwoSamplingDelay = ADC_TWOSAMPLINGDELAY_5CYCLES;
    HAL_ADCEx_MultiModeConfigChannel(&feedback_adcs[0], &mode);
}

void FeedbackADC_TimerInit(void)
{
    TIM_MasterConfigTypeDef mstrconfig;

    feedback_timer.Instance = FEEDBACK_TRIGGER_TIM;

    // Clocked from APB1 x 2, APB1 prescaler is 4, = 216MHz/2 time clock
    // (216MHz/2) / (1kHz) = 108000
    // Max prescaler is 65535, use prescaler
    feedback_timer.Init.Prescaler = 107; // prescale is value + 1
    feedback_timer.Init.Period = 1000;
    feedback_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    feedback_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    feedback_timer.Init.RepetitionCounter = 0x0;
    feedback_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&feedback_timer);

    mstrconfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    mstrconfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&feedback_timer, &mstrconfig);
}
