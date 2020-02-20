#include <math.h>
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "ads57x4.h"
#include "feedback_adc.h"

// theta = (V - Vmin) / (Vmax - Vmin) * Thetamax + Thetamin
// theta = V * Thetamax / (Vmax - Vmin) + (Thetamin - (Vmin / (Vmax - Vmin)))
#define THETA_OFFSET(Vmin, Vmax, Thetamin, Thetamax) (Thetamax / (Vmax - Vmin))
#define THETA_SCALE(Vmin, Vmax, Thetamin, Thetamax)  (Thetamin - (Vmin / (Vmax - Vmin)))

// length = sqrt(l1**2 + l2**2 + l1 * l2 * sin(theta + phi))
#define SHAPE_OFFSET(l1, l2) (l1*l1 + l2*l2)
#define SHAPE_SCALE(l1, l2) (l1*l1)

// feedback_voltage = (length - Lmin) / (Lmax - Lmin) * Vmax + Vmin
#define FEEDBACK_OFFSET(Lmin, Lmax, Vmin, Vmax) (Vmax / (Lmax - Lmin))
#define FEEDBACK_SCALE(Lmin, Lmax, Vmin, Vmax)  (Vmin - (Lmin / (Lmax - Lmin)))

static void Linearize_Thread(const void* args);

static const float ADC_VREF = 3.3f;
static const float ADC_MAX_CODE = (float)((1<<12) - 1);
static const float JOINT_DIVIDER = 2.0f / 3.0f;
static const float DAC_CODE_SCALE = 10.8f / ((1<<12)-1);

SPI_HandleTypeDef DAC_SPIHandle;
ADC_HandleTypeDef feedback_adc;

osThreadDef(lin, Linearize_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
osMessageQDef(adcdata, 2, uint32_t);
static osThreadId linearize;
static osMessageQId dataQ;

enum JointIndex {
    CURL = 0,
    LIFT = 1,
    SWING = 2,
    NJOINTS = 3
};

static const enum ads57x4_channel joint_channel[3] = {
    ADS57x4_CHANNEL_A, ADS57x4_CHANNEL_B, ADS57x4_CHANNEL_C}; 

struct LinearizationConstants {
    float theta_offset[NJOINTS];
    float theta_scale[NJOINTS];
    float shape_offset[NJOINTS];
    float shape_scale[NJOINTS];
    float shape_phase[NJOINTS];
    float feedback_offset[NJOINTS];
    float feedback_scale[NJOINTS];
};

static struct LinearizationConstants constants_storage __attribute__ ((section (".storage"))) = {
    .theta_offset = {
        THETA_OFFSET(0.88f, 4.8f, M_PI_2 - M_PI/6.0f, M_PI_2 + M_PI/6.0f),  // CURL
        THETA_OFFSET(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f),  // LIFT
        THETA_OFFSET(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f)}, // SWING
    .theta_scale = {
        THETA_SCALE(0.88f, 4.8f, M_PI_2 - M_PI/6.0f, M_PI_2 + M_PI/6.0f),   // CURL
        THETA_SCALE(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f),   // LIFT
        THETA_SCALE(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f)},  // SWING
    .shape_offset = {
        SHAPE_OFFSET(0.00f, 0.00f),                                         // CURL
        SHAPE_OFFSET(0.00f, 0.00f),                                         // LIFT
        SHAPE_OFFSET(0.00f, 0.00f)},                                        // SWING
    .shape_scale = {
        SHAPE_SCALE(0.00f, 0.00f),                                          // CURL
        SHAPE_SCALE(0.00f, 0.00f),                                          // LIFT
        SHAPE_SCALE(0.00f, 0.00f)},                                         // SWING
    .shape_phase = {0.0f, 0.0f, 0.0f},
    .feedback_offset = {
        FEEDBACK_OFFSET(0.01f, 0.08f, 0.0, 10.0f),                          // CURL
        FEEDBACK_OFFSET(0.01f, 0.08f, 0.0, 10.0f),                          // LIFT
        FEEDBACK_OFFSET(0.01f, 0.08f, 0.0, 10.0f)},                         // SWING
    .feedback_scale = {
        FEEDBACK_SCALE(0.01f, 0.08f, 0.0, 10.0f),                          // CURL
        FEEDBACK_SCALE(0.01f, 0.08f, 0.0, 10.0f),                          // LIFT
        FEEDBACK_SCALE(0.01f, 0.08f, 0.0, 10.0f)},                         // SWING
};

static struct LinearizationConstants constants;

void Linearize_ThreadInit(void)
{
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
        float voltage = (ADC_VREF * (((uint32_t)args) / ADC_MAX_CODE) / JOINT_DIVIDER);
        float angle = (constants.theta_offset[current_joint] +
                       constants.theta_scale[current_joint] * voltage);
        float length = sqrtf(constants.shape_offset[current_joint] +
                             constants.shape_scale[current_joint] * sinf(angle + constants.shape_phase[current_joint]));
        float feedback_voltage = (constants.feedback_offset[current_joint] +
                                  constants.feedback_scale[current_joint] * length);
        uint16_t feedback_code = feedback_voltage * DAC_CODE_SCALE;
        ads5724_SetVoltage(joint_channel[current_joint], feedback_code);
    }
}

void DAC_IO_TransferComplete(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
}

void DAC_IO_TransferError(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
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
    (void)hadc;
    osSignalSet(linearize, 1);
}
