#include <math.h>
#include "joint.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "ads57x4.h"
#include "feedback_adc.h"
#include "status_led.h"
#include "chomplegboard.h"

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

#define DAC_TX_COMPLETE   1
#define DAC_IO_ERROR      2
#define ADC_CONV_COMPLETE 4
#define ADC_CONV_ERROR    8

static void Linearize_Thread(const void* args);

#define __MAX_DAC_OUTPUT 10.8f
static const float ADC_VREF = 3.3f;
static const float ADC_MAX_CODE = (float)((1<<12) - 1);
static const float JOINT_DIVIDER = 2.0f / 3.0f;
static const float MAX_DAC_OUTPUT = __MAX_DAC_OUTPUT;
static const float DAC_MAX_CODE =  (float)((1<<12)-1);
static const float JOINT_ANGLE_SCALE = 1000.0f;

extern SPI_HandleTypeDef DAC_SPIHandle;
extern ADC_HandleTypeDef feedback_adcs[JOINT_COUNT];

osThreadDef(lin, Linearize_Thread, osPriorityRealtime, 1, configMINIMAL_STACK_SIZE);
osMessageQDef(adcdata, 8, uint32_t);
static osThreadId linearize;
static osMessageQId dataQ;

static uint32_t channel_values[JOINT_COUNT];
static float joint_angle[JOINT_COUNT];
static float cylinder_length[JOINT_COUNT];

/* Map joint order to ADC order */
static const uint8_t joint_adc_channel[JOINT_COUNT] = {0, 1, 2};

/* Map joint order to LED order */
static const uint8_t joint_led_channel[JOINT_COUNT] = {0, 1, 2};

/* Map joint order to DAC order */
static const enum ads57x4_channel joint_dac_channel[JOINT_COUNT] = {
    ADS57x4_CHANNEL_A, ADS57x4_CHANNEL_C, ADS57x4_CHANNEL_B};

struct LinearizationConstants {
    float theta_offset[JOINT_COUNT];
    float theta_scale[JOINT_COUNT];
    float shape_offset[JOINT_COUNT];
    float shape_scale[JOINT_COUNT];
    float shape_phase[JOINT_COUNT];
    float feedback_offset[JOINT_COUNT];
    float feedback_scale[JOINT_COUNT];
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

uint16_t Linearize_ReadAngle(void *ctx)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    return roundf(joint_angle[joint] * JOINT_ANGLE_SCALE);
}

uint16_t Linearize_ReadLength(void *ctx)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    return roundf(cylinder_length[joint] * JOINT_ANGLE_SCALE);
}


static void setup_dac(void)
{
    osEvent e;
    while(1)
    {
        ads57x4_SelectOutputRange(ADS57x4_CHANNEL_A, ADS57x4_RANGE_U10p8V);
        e = osSignalWait(DAC_TX_COMPLETE, 100);
        if(e.status != osEventSignal || e.value.signals != DAC_TX_COMPLETE)
            continue;
        ads57x4_SelectOutputRange(ADS57x4_CHANNEL_B, ADS57x4_RANGE_U10p8V);
        e = osSignalWait(DAC_TX_COMPLETE, 100);
        if(e.status != osEventSignal || e.value.signals != DAC_TX_COMPLETE)
            continue;
        ads57x4_SelectOutputRange(ADS57x4_CHANNEL_C, ADS57x4_RANGE_U10p8V);
        e = osSignalWait(DAC_TX_COMPLETE, 100);
        if(e.status != osEventSignal || e.value.signals != DAC_TX_COMPLETE)
            continue;
        ads57x4_Configure(true, false, ADS57x4_CLEAR_ZERO, false);
        e = osSignalWait(DAC_TX_COMPLETE, 100);
        if(e.status != osEventSignal || e.value.signals != DAC_TX_COMPLETE)
            continue;
        ads57x4_Power(7);
        e = osSignalWait(DAC_TX_COMPLETE, 100);
        if(e.status != osEventSignal || e.value.signals != DAC_TX_COMPLETE)
            continue;
        break;
    }
}

static void adc_reinit(void)
{
    HAL_TIM_Base_DeInit(&feedback_timer);

    /* Done in ADC order */
    HAL_ADC_DeInit(&feedback_adcs[0]);
    HAL_ADC_DeInit(&feedback_adcs[1]);
    HAL_ADC_DeInit(&feedback_adcs[2]);

    FeedbackADC_TimerInit();

    FeedbackADC_Init();

    HAL_ADC_Start(&feedback_adcs[2]);
    HAL_ADC_Start(&feedback_adcs[1]);
    HAL_ADCEx_MultiModeStart_DMA(&feedback_adcs[0], channel_values, 3);
    HAL_TIM_Base_Start(&feedback_timer);
}

static void Linearize_Thread(const void* args)
{
    (void)args;
    osEvent event;
    float voltage[JOINT_COUNT];
    float feedback_voltage[JOINT_COUNT];
    uint16_t feedback_code[JOINT_COUNT];
    float brightness[JOINT_COUNT];
    int sendjoint=3;

    setup_dac();
    HAL_ADC_Start(&feedback_adcs[2]);
    HAL_ADC_Start(&feedback_adcs[1]);
    HAL_ADCEx_MultiModeStart_DMA(&feedback_adcs[0], channel_values, 3);
    HAL_TIM_Base_Start(&feedback_timer);
    for(;;)
    {
        event = osSignalWait(ADC_CONV_COMPLETE | ADC_CONV_ERROR | DAC_TX_COMPLETE, osWaitForever);
        if(event.status != osEventSignal)
        {
            continue;
        }
        if(event.value.signals & ADC_CONV_ERROR)
        {
                adc_reinit();
        }
        else if(event.value.signals & DAC_TX_COMPLETE)
        {
            if(sendjoint<3)
            {
                ads5724_SetVoltage(joint_dac_channel[sendjoint], feedback_code[sendjoint]);
                sendjoint++;
            }
            else
            {
                DAC_IO_LDAC(true);
            }
        }
        else if(event.value.signals & ADC_CONV_COMPLETE)
        {
            DAC_IO_LDAC(false);
            for(int joint=0;joint<JOINT_COUNT;joint++)
            {
                voltage[joint] = (ADC_VREF * (channel_values[joint_adc_channel[joint]] / ADC_MAX_CODE) * JOINT_DIVIDER);
                joint_angle[joint] = (linearization_constants.theta_offset[joint] +
                                      linearization_constants.theta_scale[joint] * voltage[joint]);
            }
            // float length = sqrtf(linearization_constants.shape_offset[current_joint] +
            //                      linearization_constants.shape_scale[current_joint] * sinf(angle + linearization_constants.shape_phase[current_joint]));
            // float feedback_voltage = (linearization_constants.feedback_offset[current_joint] +
            //                           linearization_constants.feedback_scale[current_joint] * length);
            for(int joint=0;joint<JOINT_COUNT;joint++)
            {
                feedback_voltage[joint] = MAX_DAC_OUTPUT - voltage[joint] / JOINT_DIVIDER / ADC_VREF * MAX_DAC_OUTPUT;
                feedback_code[joint] = feedback_voltage[joint] * DAC_MAX_CODE / MAX_DAC_OUTPUT;
                brightness[joint] = voltage[joint]*250.0f / (ADC_VREF * JOINT_DIVIDER);
                brightness[joint] = (brightness[joint] > 255.0f) ? 255.0f : ((brightness[joint] < 0.0f) ? 0.0f : brightness[joint]);
                LED_SetOne(joint_led_channel[joint], 1, brightness[joint]);
            }
            sendjoint = 0;
            ads5724_SetVoltage(joint_dac_channel[sendjoint], feedback_code[sendjoint]);
            sendjoint++;
        }
    }
}

void DAC_IO_TransmitComplete(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
    osSignalSet(linearize, DAC_TX_COMPLETE);
}

void DAC_IO_Error(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
    osSignalSet(linearize, DAC_IO_ERROR);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    osSignalSet(linearize, ADC_CONV_COMPLETE);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    (void)hadc;
    osSignalSet(linearize, ADC_CONV_ERROR);
}
