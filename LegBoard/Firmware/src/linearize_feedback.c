#include <math.h>
#include "main.h"
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

extern SPI_HandleTypeDef DAC_SPIHandle;
extern ADC_HandleTypeDef feedback_adcs[3];

osThreadDef(lin, Linearize_Thread, osPriorityHigh, 0, configMINIMAL_STACK_SIZE);
osMessageQDef(adcdata, 8, uint32_t);
static osThreadId linearize;
static osMessageQId dataQ;

static uint32_t channel_values[3];

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

void Linearize_Thread(const void* args)
{
    (void)args;
    osEvent event;
    float voltage[3];
    float feedback_voltage[3];
    uint16_t feedback_code[3];
    float brightness[3];
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
                ads5724_SetVoltage(joint_channel[sendjoint], feedback_code[sendjoint]);
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
            voltage[0] = (ADC_VREF * (channel_values[0] / ADC_MAX_CODE) * JOINT_DIVIDER);
            voltage[1] = (ADC_VREF * (channel_values[1] / ADC_MAX_CODE) * JOINT_DIVIDER);
            voltage[2] = (ADC_VREF * (channel_values[2] / ADC_MAX_CODE) * JOINT_DIVIDER);
            // float angle = (linearization_constants.theta_offset[current_joint] +
            //                linearization_constants.theta_scale[current_joint] * voltage);
            // float length = sqrtf(linearization_constants.shape_offset[current_joint] +
            //                      linearization_constants.shape_scale[current_joint] * sinf(angle + linearization_constants.shape_phase[current_joint]));
            // float feedback_voltage = (linearization_constants.feedback_offset[current_joint] +
            //                           linearization_constants.feedback_scale[current_joint] * length);
            feedback_voltage[0] = MAX_DAC_OUTPUT - voltage[0] / JOINT_DIVIDER / ADC_VREF * MAX_DAC_OUTPUT;
            feedback_voltage[1] = MAX_DAC_OUTPUT - voltage[1] / JOINT_DIVIDER / ADC_VREF * MAX_DAC_OUTPUT;
            feedback_voltage[2] = MAX_DAC_OUTPUT - voltage[2] / JOINT_DIVIDER / ADC_VREF * MAX_DAC_OUTPUT;
            feedback_code[0] = feedback_voltage[0] * DAC_MAX_CODE / MAX_DAC_OUTPUT;
            feedback_code[1] = feedback_voltage[1] * DAC_MAX_CODE / MAX_DAC_OUTPUT;
            feedback_code[2] = feedback_voltage[2] * DAC_MAX_CODE / MAX_DAC_OUTPUT;
            sendjoint = 0;
            ads5724_SetVoltage(joint_channel[sendjoint], feedback_code[sendjoint]);
            sendjoint++;
            brightness[0] = voltage[0]*250.0f / (ADC_VREF * JOINT_DIVIDER);
            brightness[0] = (brightness[0] > 255.0f) ? 255.0f : ((brightness[0] < 0.0f) ? 0.0f : brightness[0]);
            brightness[1] = voltage[1]*250.0f / (ADC_VREF * JOINT_DIVIDER);
            brightness[1] = (brightness[1] > 255.0f) ? 255.0f : ((brightness[1] < 0.0f) ? 0.0f : brightness[1]);
            brightness[2] = voltage[2]*250.0f / (ADC_VREF * JOINT_DIVIDER);
            brightness[2] = (brightness[2] > 255.0f) ? 255.0f : ((brightness[2] < 0.0f) ? 0.0f : brightness[2]);
            LED_SetOne(0, 1, brightness[0]);
            LED_SetOne(1, 1, brightness[1]);
            LED_SetOne(2, 1, brightness[2]);
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
