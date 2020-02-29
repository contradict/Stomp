#include <math.h>
#include "main.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "ads57x4.h"
#include "feedback_adc.h"
#include "status_led.h"

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

#define __MAX_DAC_OUTPUT 10.8f
static const float ADC_VREF = 3.3f;
static const float ADC_MAX_CODE = (float)((1<<12) - 1);
static const float JOINT_DIVIDER = 2.0f / 3.0f;
static const float MAX_DAC_OUTPUT = __MAX_DAC_OUTPUT;
static const float DAC_MAX_CODE =  (float)((1<<12)-1);

extern SPI_HandleTypeDef DAC_SPIHandle;
ADC_HandleTypeDef feedback_adc;

osThreadDef(lin, Linearize_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
osMessageQDef(adcdata, 2, uint32_t);
static osThreadId linearize;
static osMessageQId dataQ;

static const enum ads57x4_channel joint_channel[3] = {
    ADS57x4_CHANNEL_A, ADS57x4_CHANNEL_C, ADS57x4_CHANNEL_B};

struct LinearizationConstants {
    float theta_offset[NJOINTS];
    float theta_scale[NJOINTS];
    float shape_offset[NJOINTS];
    float shape_scale[NJOINTS];
    float shape_phase[NJOINTS];
    float feedback_offset[NJOINTS];
    float feedback_scale[NJOINTS];
};

static struct LinearizationConstants linearization_constants __attribute__ ((section (".storage.linearize"))) = {
    .theta_offset = {
        THETA_OFFSET(0.88f, 4.8f, M_PI_2 - M_PI/6.0f, M_PI_2 + M_PI/6.0f),  // CURL
        THETA_OFFSET(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f),  // SWING
        THETA_OFFSET(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f)}, // LIFT
    .theta_scale = {
        THETA_SCALE(0.88f, 4.8f, M_PI_2 - M_PI/6.0f, M_PI_2 + M_PI/6.0f),   // CURL
        THETA_SCALE(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f),   // SWING
        THETA_SCALE(0.88f, 4.8f, M_PI_2 - M_PI/8.0f, M_PI_2 + M_PI/8.0f)},  // LIFT
    .shape_offset = {
        SHAPE_OFFSET(0.00f, 0.00f),                                         // CURL
        SHAPE_OFFSET(0.00f, 0.00f),                                         // SWING
        SHAPE_OFFSET(0.00f, 0.00f)},                                        // LIFT
    .shape_scale = {
        SHAPE_SCALE(0.00f, 0.00f),                                          // CURL
        SHAPE_SCALE(0.00f, 0.00f),                                          // SWING
        SHAPE_SCALE(0.00f, 0.00f)},                                         // LIFT
    .shape_phase = {0.0f, 0.0f, 0.0f},
    .feedback_offset = {
        FEEDBACK_OFFSET(0.01f, 0.08f, 0.0, 10.0f),                          // CURL
        FEEDBACK_OFFSET(0.01f, 0.08f, 0.0, 10.0f),                          // SWING
        FEEDBACK_OFFSET(0.01f, 0.08f, 0.0, 10.0f)},                         // LIFT
    .feedback_scale = {
        FEEDBACK_SCALE(0.01f, 0.08f, 0.0, 10.0f),                          // CURL
        FEEDBACK_SCALE(0.01f, 0.08f, 0.0, 10.0f),                          // SWING
        FEEDBACK_SCALE(0.01f, 0.08f, 0.0, 10.0f)},                         // LIFT
};

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

static void setup_dac(void)
{
    osEvent e;
    while(1)
    {
        ads57x4_SelectOutputRange(ADS57x4_CHANNEL_A, ADS57x4_RANGE_U10p8V);
        e = osSignalWait(0, 100);
        if(e.status != osEventSignal || e.value.signals != 0)
            continue;
        ads57x4_SelectOutputRange(ADS57x4_CHANNEL_B, ADS57x4_RANGE_U10p8V);
        e = osSignalWait(0, 100);
        if(e.status != osEventSignal || e.value.signals != 0)
            continue;
        ads57x4_SelectOutputRange(ADS57x4_CHANNEL_C, ADS57x4_RANGE_U10p8V);
        e = osSignalWait(0, 100);
        if(e.status != osEventSignal || e.value.signals != 0)
            continue;
        ads57x4_Configure(true, false, ADS57x4_CLEAR_ZERO, false);
        e = osSignalWait(0, 100);
        if(e.status != osEventSignal || e.value.signals != 0)
            continue;
        ads57x4_Power(7);
        e = osSignalWait(0, 100);
        if(e.status != osEventSignal || e.value.signals != 0)
            continue;
        break;
    }
}

static void adc_reinit(void)
{
    HAL_TIM_Base_DeInit(&feedback_timer);

    HAL_ADC_DeInit(&feedback_adc);

    FeedbackADC_TimerInit();

    FeedbackADC_Init();

    HAL_ADC_Start_IT(&feedback_adc);
    HAL_TIM_Base_Start(&feedback_timer);
}

void Linearize_Thread(const void* args)
{
    (void)args;
    osEvent event;
    setup_dac();
    HAL_ADC_Start_IT(&feedback_adc);
    HAL_TIM_Base_Start(&feedback_timer);
    for(enum JointIndex current_joint = CURL;
        true;
        current_joint = (current_joint + 1) % 3)
    {
        event = osSignalWait(0, 0);
        if(event.status == osEventSignal && event.value.v != 0)
        {
            current_joint = LIFT;
            adc_reinit();
            continue;
        }
        event = osMessageGet(dataQ, osWaitForever);
        if(event.status != osEventMessage)
        {
            current_joint = LIFT;
            adc_reinit();
            continue;
        }
        float voltage = (ADC_VREF * (event.value.v / ADC_MAX_CODE) * JOINT_DIVIDER);
        // float angle = (linearization_constants.theta_offset[current_joint] +
        //                linearization_constants.theta_scale[current_joint] * voltage);
        // float length = sqrtf(linearization_constants.shape_offset[current_joint] +
        //                      linearization_constants.shape_scale[current_joint] * sinf(angle + linearization_constants.shape_phase[current_joint]));
        // float feedback_voltage = (linearization_constants.feedback_offset[current_joint] +
        //                           linearization_constants.feedback_scale[current_joint] * length);
        float feedback_voltage = MAX_DAC_OUTPUT - voltage / JOINT_DIVIDER / ADC_VREF * MAX_DAC_OUTPUT;
        uint16_t feedback_code = feedback_voltage * DAC_MAX_CODE / MAX_DAC_OUTPUT;
        ads5724_SetVoltage(joint_channel[current_joint], feedback_code<<4);
        float brightness = voltage*250.0f / (ADC_VREF * JOINT_DIVIDER);
        brightness = (brightness > 255.0f) ? 255.0f : ((brightness < 0.0f) ? 0.0f : brightness);
        LED_G(current_joint, brightness);
    }
}

void DAC_IO_TransmitComplete(SPI_HandleTypeDef *hspi)
{
    __HAL_SPI_DISABLE(hspi);
    osSignalSet(linearize, 0);
}

void DAC_IO_Error(SPI_HandleTypeDef *hspi)
{
    __HAL_SPI_DISABLE(hspi);
    osSignalSet(linearize, 1);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if(osOK != osMessagePut(dataQ, hadc->Instance->DR, 0))
    {
        osSignalSet(linearize, 2);
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    osSignalSet(linearize, 1);
}
