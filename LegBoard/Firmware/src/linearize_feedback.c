#include <math.h>
#include <complex.h>
#include "joint.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "ads57x4.h"
#include "feedback_adc.h"
#include "status_led.h"
#include "chomplegboard.h"
#include "linearize_feedback.h"
#include "modbus.h"

// theta = (V - Vmin) / (Vmax - Vmin) * (Thetamax - Thetamin) + Thetamin
// theta = V * (Thetamax - Thetamin) / (Vmax - Vmin) + (Thetamin - (Vmin * (Thetamax - Thetamin) / (Vmax - Vmin)))
#define THETA_SCALE(Vmin, Vmax, Thetamin, Thetamax) (((Thetamax) - (Thetamin)) / ((Vmax) - (Vmin)))
#define THETA_OFFSET(Vmin, Vmax, Thetamin, Thetamax)  ((Thetamin) - Vmin * ((Thetamax) - (Thetamin)) / ((Vmax) - (Vmin)))

// length = sqrt(l1**2 + l2**2 - l1 * l2 * cos(theta + phi))
#define SHAPE_OFFSET(l1, l2) (l1*l1 + l2*l2)
#define SHAPE_SCALE(l1, l2) (l1*l1)

// feedback_voltage = (length - Lmin) / (Lmax - Lmin) * (Vmax - Vmin) + Vmin
//                  = length * scale + offset
#define FEEDBACK_SCALE(Lmin, Lmax, Vmin, Vmax) (((Vmax) - (Vmin)) / ((Lmax) - (Lmin)))
#define FEEDBACK_OFFSET(Lmin, Lmax, Vmin, Vmax)  ((Vmin) - (Lmin) * ((Vmax) - (Vmin)) / ((Lmax) - (Lmin)))

#define DAC_TX_COMPLETE   1
#define DAC_IO_ERROR      2
#define ADC_CONV_COMPLETE 4
#define ADC_CONV_ERROR    8

static void Linearize_Thread(const void* args);
void compute_joint_angles(const uint32_t channel_values[JOINT_COUNT],
                          float voltage[JOINT_COUNT],
                          float joint_angle[JOINT_COUNT]);
void compute_cylinder_edge_lengths(const float joint_angle[JOINT_COUNT],
                                   float cylinder_edge_length[JOINT_COUNT]);
void compute_feedback_voltage(const float cylinder_length[JOINT_COUNT],
                              float feedback_voltage[JOINT_COUNT],
                              uint16_t feedback_code[JOINT_COUNT]);
void compute_led_brightness(const float joint_voltage[JOINT_COUNT]);

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
static float cylinder_edge_length[JOINT_COUNT];

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
    complex float Pl;
    float L1, L2;
    float Rllimit[2];
    complex float Pc;
    float C1, C2;
    float Rclimit[2];
    float S1;
    complex float Ps;
    float Rslimit[2];
};

static struct LinearizationConstants linearization_constants __attribute__ ((section (".storage.linearize"))) = {
    .theta_offset = {
      //THETA_OFFSET(Vmin,     Vmax,            Thetamin,           Thetamax)
        THETA_OFFSET(0.916f, 4.774f, -M_PI_2 - M_PI/6.0f, -M_PI_2 + M_PI/6.0f),  // CURL
        THETA_OFFSET(0.225f, 4.768f,          -M_PI/8.0f,           M_PI/8.0f),  // SWING
        THETA_OFFSET(1.009f, 3.637f,          -M_PI/8.0f,           M_PI/8.0f)}, // LIFT
    .theta_scale = {
        THETA_SCALE(0.916f, 4.774f, -M_PI_2 - M_PI/6.0f, -M_PI_2 + M_PI/6.0f),   // CURL
        THETA_SCALE(0.225f, 4.768f,          -M_PI/8.0f,           M_PI/8.0f),   // SWING
        THETA_SCALE(1.009f, 3.637f,          -M_PI/8.0f,           M_PI/8.0f)},  // LIFT
    .Pl = 4.287 + 2.06I,
    .L1 = 4.4,
    .L2 = 6.3,
    .Rllimit = {1.07, 3.05},
    .Pc = 4.287 - 0.44I,
    .C1 = 2.5,
    .C2 = 6.0,
    .Rclimit = {5.8, 6.8},
    .S1 = 3.90,
    .Ps = 3.603 + 2.572I,
    .Rslimit = {1.080, 2.572}
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

int Linearize_ReadAngle(void *ctx, uint16_t *v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    *v = roundf(joint_angle[joint] * JOINT_ANGLE_SCALE);
    return 0;
}

int Linearize_ReadLength(void *ctx, uint16_t *v)
{
    enum JointIndex joint = (enum JointIndex)ctx;
    if((joint < 0) || (joint >= JOINT_COUNT))
    {
        return ILLEGAL_DATA_ADDRESS;
    }
    *v = roundf(cylinder_edge_length[joint] * JOINT_ANGLE_SCALE);
    return 0;
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
    float joint_voltage[JOINT_COUNT];
    float feedback_voltage[JOINT_COUNT];
    uint16_t feedback_code[JOINT_COUNT];
    int sendjoint=JOINT_COUNT; // start past end, need to reset to start sending

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
            if(sendjoint<JOINT_COUNT)
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
            compute_joint_angles(channel_values, joint_voltage, joint_angle);
            compute_cylinder_edge_lengths(joint_angle, cylinder_edge_length);
            compute_feedback_voltage(cylinder_edge_length, feedback_voltage, feedback_code);
            compute_led_brightness(joint_voltage);
            sendjoint = 0;
            ads5724_SetVoltage(joint_dac_channel[sendjoint], feedback_code[sendjoint]);
            sendjoint++;
        }
    }
}

void compute_joint_angles(const uint32_t channel_values[JOINT_COUNT],
                          float voltage[JOINT_COUNT],
                          float joint_angle[JOINT_COUNT])
{
    for(enum JointIndex joint=0;joint<JOINT_COUNT;joint++)
    {
        voltage[joint] = (ADC_VREF * (channel_values[joint_adc_channel[joint]] / ADC_MAX_CODE) / JOINT_DIVIDER);
        joint_angle[joint] = (linearization_constants.theta_offset[joint] +
                linearization_constants.theta_scale[joint] * voltage[joint]);
    }
}

void compute_cylinder_edge_lengths(const float joint_angle[JOINT_COUNT],
                                   float cylinder_edge_length[JOINT_COUNT])
{
    cylinder_edge_length[JOINT_LIFT] = cabsf(
            linearization_constants.Pl -
            linearization_constants.L1 * cexpf(I * joint_angle[JOINT_LIFT]));
    cylinder_edge_length[JOINT_CURL] = cabsf(
            linearization_constants.Pl - linearization_constants.Pc +
            linearization_constants.C1 * cexpf(I * (joint_angle[JOINT_CURL] + joint_angle[JOINT_LIFT])) +
            linearization_constants.L2 * cexpf(I * joint_angle[JOINT_LIFT]));
    cylinder_edge_length[JOINT_SWING] = cabsf(
            linearization_constants.Ps - linearization_constants.S1 * cexpf(I * joint_angle[JOINT_SWING]));
}

void compute_feedback_voltage(const float cylinder_edge_length[JOINT_COUNT],
                              float feedback_voltage[JOINT_COUNT],
                              uint16_t feedback_code[JOINT_COUNT])
{
    float scale[3], offset[3];
    scale[JOINT_CURL] =  FEEDBACK_SCALE(linearization_constants.Rclimit[0],
                                        linearization_constants.Rclimit[1],
                                        0.0f, 10.0f);
    scale[JOINT_SWING] =  FEEDBACK_SCALE(linearization_constants.Rslimit[0],
                                         linearization_constants.Rslimit[1],
                                         0.0f, 10.0f);
    scale[JOINT_LIFT] =  FEEDBACK_SCALE(linearization_constants.Rllimit[0],
                                        linearization_constants.Rllimit[1],
                                        0.0f, 10.0f);
    offset[JOINT_CURL] =  FEEDBACK_OFFSET(linearization_constants.Rclimit[0],
                                          linearization_constants.Rclimit[1],
                                          0.0f, 10.0f);
    offset[JOINT_SWING] =  FEEDBACK_OFFSET(linearization_constants.Rslimit[0],
                                           linearization_constants.Rslimit[1],
                                           0.0f, 10.0f);
    offset[JOINT_LIFT] =  FEEDBACK_OFFSET(linearization_constants.Rllimit[0],
                                          linearization_constants.Rllimit[1],
                                          0.0f, 10.0f);

    for(int joint = 0; joint < JOINT_COUNT; joint++)
    {
        feedback_voltage[joint] = (scale[joint] * cylinder_edge_length[joint] + offset[joint]);
        feedback_code[joint] = feedback_voltage[joint] * DAC_MAX_CODE / MAX_DAC_OUTPUT;
    }
}

void compute_led_brightness(const float joint_voltage[JOINT_COUNT])
{
    float brightness;
    for(int joint=0;joint<JOINT_COUNT;joint++)
    {
        brightness = joint_voltage[joint] * JOINT_DIVIDER / ADC_VREF * 250.0f;
        brightness = (brightness > 255.0f) ? 255.0f : ((brightness < 0.0f) ? 0.0f : brightness);
        LED_SetOne(joint_led_channel[joint], 1, brightness);
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
