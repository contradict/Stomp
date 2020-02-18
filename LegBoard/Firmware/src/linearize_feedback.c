#include <math.h>
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "ads57x4.h"
#include "feedback_adc.h"

/*
#define M_PI    3.141592653589793238462643383279502884f
#define M_PI_2  1.570796326794896619231321691639751442f
#define M_PI_4  0.785398163397448309615660845819875721f
*/

static const float ADC_VREF = 3.3f;
static const float ADC_MAX_CODE = (float)((1<<12) - 1);
static const float JOINT_DIVIDER = 2.0f / 3.0f;
static const float DAC_CODE_SCALE = 10.8f / ((1<<12)-1);

SPI_HandleTypeDef DAC_SPIHandle;
ADC_HandleTypeDef feedback_adc;

static osThreadId linearize;
static osMessageQId dataQ;

static void Linearize_Thread(const void* args);

enum JointIndex {
    CURL = 0,
    LIFT = 1,
    SWING = 2,
    NJOINTS = 3
};

static const enum ads57x4_channel joint_channel[3] = {
    ADS57x4_CHANNEL_A, ADS57x4_CHANNEL_B, ADS57x4_CHANNEL_C}; 

static struct LinearizationConstants {
    float theta_offset[NJOINTS];
    float theta_scale[NJOINTS];
    float shape_offset[NJOINTS];
    float shape_scale[NJOINTS];
    float shape_phase[NJOINTS];
    float feedback_offset[NJOINTS];
    float feedback_scale[NJOINTS];
} constants __attribute__ ((section ("storage"))) = {
    .theta_offset = {M_PI_2 - ((4.8 + 0.8) / 2),
                     M_PI_2 - ((4.8 + 0.8) / 2),
                     M_PI_2 - ((4.8 + 0.8) / 2)},
    .theta_scale = {M_PI_4 / (4.8 - 0.8),
                    1.0f, 1.0f}
};

void Linearize_ThreadInit(void)
{
    osThreadDef(lin, Linearize_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osMessageQDef(adcdata, 2, uint32_t);
    /* Configure on-board ADC to read Feedback signals in repeating sequence */
    /* CURL, LIFT, SWING */
    /* Runs at 3kHz, so each channel sees 1kHz updates */
    ads57x4_Init();

    FeedbackADC_TimerInit();  

    FeedbackADC_Init();

    linearize = osThreadCreate(osThread(lin), NULL);
    dataQ = osMessageCreate(osMessageQ(adcdata), linearize);
}

void Linearize_Thread(const void* args)
{
    osEvent event;
    HAL_ADC_Start_IT(&feedback_adc);
    for(enum JointIndex current_joint = CURL;
        true;
        current_joint = (current_joint + 1) % 3)
    {
        event = osSignalWait(0, 0);
        if(event.status == osEventSignal && event.value.v != 0)
        {
            current_joint = CURL;
            HAL_ADC_Stop_IT(&feedback_adc);
            HAL_ADC_Start_IT(&feedback_adc);
            continue;
        }
        event = osMessageGet(dataQ, osWaitForever);
        if(event.status != osEventMessage)
        {
            current_joint = CURL;
            HAL_ADC_Stop_IT(&feedback_adc);
            HAL_ADC_Start_IT(&feedback_adc);
            continue;
        }
        float voltage = (ADC_VREF * (event.value.v / ADC_MAX_CODE) / JOINT_DIVIDER);
        float angle = (constants.theta_offset[current_joint] +
                       constants.theta_scale[current_joint] * voltage);
        float length = (constants.shape_offset[current_joint] +
                        constants.shape_scale[current_joint] * sinf(angle + constants.shape_phase[current_joint]));
        float feedback_voltage = (constants.feedback_offset[current_joint] +
                                  constants.feedback_scale[current_joint] * length);
        uint16_t feedback_code = feedback_voltage * DAC_CODE_SCALE;
        ads5724_SetVoltage(joint_channel[current_joint], feedback_code);
    }
}

void DAC_IO_TransferComplete(SPI_HandleTypeDef *hspi)
{
}

void DAC_IO_TransferError(SPI_HandleTypeDef *hspi)
{
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(osOK != osMessagePut(linearize, hadc->Instance->DR, 1))
    {
        osSignalSet(linearize, 1);
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    osSignalSet(linearize, 1);
}
