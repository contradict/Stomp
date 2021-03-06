#include <math.h>
#include <string.h>
#include "export/joint.h"
#include "cmsis_os.h"
#include "stm32f7xx_hal.h"
#include "ads57x4.h"
#include "feedback_adc.h"
#include "status_led.h"
#include "chomplegboard.h"
#include "linearize_feedback.h"
#include "modbus.h"
#include "kinematics.h"

// theta = (V - Vmin) / (Vmax - Vmin) * (Thetamax - Thetamin) + Thetamin
// theta = V * (Thetamax - Thetamin) / (Vmax - Vmin) + (Thetamin - (Vmin * (Thetamax - Thetamin) / (Vmax - Vmin)))
#define THETA_SCALE(Vmin, Vmax, Thetamin, Thetamax) (((Thetamax) - (Thetamin)) / ((Vmax) - (Vmin)))
#define THETA_OFFSET(Vmin, Vmax, Thetamin, Thetamax)  ((Thetamin) - (Vmin) * ((Thetamax) - (Thetamin)) / ((Vmax) - (Vmin)))

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

#define LED_RATE_DIVISOR 30

static void Linearize_ConstantInit(void);
static void Linearize_Thread(const void* args);
static void lowpass_sensor_readings(const uint32_t channel_values[JOINT_COUNT],
                                    float voltage[JOINT_COUNT]);
static void compute_joint_angles(float voltage[JOINT_COUNT],
                                 float joint_angle[JOINT_COUNT]);
static void Linearize_ComputeFeedback(const float cylinder_length[JOINT_COUNT],
                                      float feedback_voltage[JOINT_COUNT],
                                      uint16_t feedback_code[JOINT_COUNT]);
static void compute_led_brightness(const float joint_voltage[JOINT_COUNT]);
static void Linearize_ScaleCylinders(const float cylinder_edge_length[JOINT_COUNT],
                                     float scaled_values[JOINT_COUNT]);

static const float ADC_SAMPLE_PERIOD = 1e-3f;
#define __MAX_DAC_OUTPUT 10.8f
static const float ADC_VREF = 3.3f;
static const float ADC_MAX_CODE = (float)((1<<12) - 1);
static const float JOINT_DIVIDER = 2.0f / 3.0f;
static const float MAX_DAC_OUTPUT = __MAX_DAC_OUTPUT;
static const float MAX_ENFIELD_SCALE = 10.0f / __MAX_DAC_OUTPUT;
static const float DAC_MAX_CODE =  (float)((1<<12)-1);
static const float JOINT_ANGLE_SCALE = 1000.0f;
static const float VOLTAGE_SCALE = 1000.0f;
static const float CYLINDER_LENGTH_SCALE = 10000.0f;

extern SPI_HandleTypeDef DAC_SPIHandle;
extern ADC_HandleTypeDef feedback_adcs[JOINT_COUNT];

osThreadDef(lin, Linearize_Thread, osPriorityRealtime, 1, configMINIMAL_STACK_SIZE);
osMessageQDef(adcdata, 8, uint32_t);
static osThreadId linearize;
static osMessageQId dataQ;

static uint32_t channel_values[JOINT_COUNT];
static float sensor_voltage[JOINT_COUNT];
static float joint_angle[JOINT_COUNT];
static float cylinder_edge_length[JOINT_COUNT];
static float cylinder_scaled_values[JOINT_COUNT];
static float feedback_voltage[JOINT_COUNT];

/* Map joint order to ADC order */
static const uint8_t joint_adc_channel[JOINT_COUNT] = {0, 1, 2};

/* Map joint order to LED order */
static const uint8_t joint_led_channel[JOINT_COUNT] = {0, 1, 2};

/* Map joint order to DAC order */
static const enum ads57x4_channel joint_dac_channel[JOINT_COUNT] = {
    ADS57x4_CHANNEL_A, ADS57x4_CHANNEL_C, ADS57x4_CHANNEL_B};

struct SensorCalibrationStorage {
    float v_min[JOINT_COUNT];
    float v_max[JOINT_COUNT];
    float theta_min[JOINT_COUNT];
    float theta_max[JOINT_COUNT];
    float cylinder_length_min[JOINT_COUNT];
    float cylinder_length_max[JOINT_COUNT];
    float feedback_lowpass_frequency[JOINT_COUNT];
};

struct SensorCalibrationProcessed {
    float theta_offset[JOINT_COUNT];
    float theta_scale[JOINT_COUNT];
    float feedback_lowpass_constant[JOINT_COUNT];
    float cylinder_offset[JOINT_COUNT];
    float cylinder_scale[JOINT_COUNT];
};

static struct SensorCalibrationStorage calibration_constants_stored __attribute__ ((section (".storage.linearize"))) = {
    .v_min = {
        0.900f, // CURL
        0.250f, // SWING
        1.010f, // LIFT
    },

    .v_max = {
        4.800f, // CURL
        4.800f, // SWING
        3.660f, // LIFT
    },

    .theta_min = {
        -M_PI_2-25.1*M_PI/180.0f, // CURL
        -M_PI/8.0f,               // SWING
        -13*M_PI/180.0f,          // LIFT
    },

    .theta_max = {
        -M_PI_2+25.1*M_PI/180.0f, // CURL
        M_PI/8.0f,                // SWING
        13*M_PI/180.0f,           // LIFT
    },

    .cylinder_length_min = {
        0.14732f, // CURL
        0.027432f, // SWING
        0.027178f, // LIFT
    },

    .cylinder_length_max = {
        0.17272f, // CURL
        0.103251f, // SWING
        0.07747f, // LIFT
    },

    .feedback_lowpass_frequency = { // 0 - disable lowpass, otherwise Hz
        0.0,
        0.0,
        0.0,
    },
};

static struct SensorCalibrationProcessed calibration_constants_processed;

void Linearize_ThreadInit(void)
{
    /* Configure on-board ADC to read Feedback signals in repeating sequence */
    /* CURL, LIFT, SWING */
    /* Runs at 3kHz, so each channel sees 1kHz updates */
    ads57x4_Init();

    FeedbackADC_TimerInit();

    FeedbackADC_Init();

    Linearize_ConstantInit();

    linearize = osThreadCreate(osThread(lin), NULL);
    dataQ = osMessageCreate(osMessageQ(adcdata), linearize);
}

#define GETJOINT(x) \
    enum JointIndex x = (enum JointIndex)ctx; \
    if((x < 0) || (x >= JOINT_COUNT)) \
    { \
        return ILLEGAL_DATA_ADDRESS; \
    }

static void Linearize_ConstantInit(void)
{
    float f;
    for(int j=0; j<JOINT_COUNT; j++)
    {
        calibration_constants_processed.theta_offset[j] = THETA_OFFSET(calibration_constants_stored.v_min[j], calibration_constants_stored.v_max[j], calibration_constants_stored.theta_min[j], calibration_constants_stored.theta_max[j]);
        calibration_constants_processed.theta_scale[j] = THETA_SCALE(calibration_constants_stored.v_min[j], calibration_constants_stored.v_max[j], calibration_constants_stored.theta_min[j], calibration_constants_stored.theta_max[j]);
        calibration_constants_processed.cylinder_scale[j] = FEEDBACK_SCALE(
                calibration_constants_stored.cylinder_length_min[j],
                calibration_constants_stored.cylinder_length_max[j],
                0.0f, MAX_ENFIELD_SCALE);
        calibration_constants_processed.cylinder_offset[j] = FEEDBACK_OFFSET(
                calibration_constants_stored.cylinder_length_min[j],
                calibration_constants_stored.cylinder_length_max[j],
                0.0f, MAX_ENFIELD_SCALE);
        if(isnan(calibration_constants_stored.feedback_lowpass_frequency[j]) ||
           calibration_constants_stored.feedback_lowpass_frequency[j] == 0.0f)
        {
            calibration_constants_processed.feedback_lowpass_constant[j] = 1.0f;
        }
        else
        {
            f = 2 * M_PI * ADC_SAMPLE_PERIOD * calibration_constants_stored.feedback_lowpass_frequency[j];
            calibration_constants_processed.feedback_lowpass_constant[j] = f / (f + 1.0f);
        }
    }
}

void Linearize_GetJointAngles(float a[3])
{
    memcpy(a, joint_angle, sizeof(joint_angle));
}

int Linearize_GetSensorVmin(void *ctx, uint16_t *vmin)
{
    GETJOINT(joint);
    *(int16_t *)vmin = roundf(calibration_constants_stored.v_min[joint] * VOLTAGE_SCALE);
    return 0;
}

int Linearize_GetSensorVmax(void *ctx, uint16_t *vmax)
{
    GETJOINT(joint);
    *(int16_t *)vmax = roundf(calibration_constants_stored.v_max[joint] * VOLTAGE_SCALE);
    return 0;
}

int Linearize_GetSensorThetamin(void *ctx, uint16_t *tmin)
{
    GETJOINT(joint);
    *(int16_t *)tmin = roundf(calibration_constants_stored.theta_min[joint] * JOINT_ANGLE_SCALE);
    return 0;
}

int Linearize_GetSensorThetamax(void *ctx, uint16_t *tmax)
{
    GETJOINT(joint);
    *(int16_t *)tmax = roundf(calibration_constants_stored.theta_max[joint] * JOINT_ANGLE_SCALE);
    return 0;
}

int Linearize_GetCylinderLengthMin(void *ctx, uint16_t *lmin)
{
    GETJOINT(joint);
    *(int16_t *)lmin = roundf(calibration_constants_stored.cylinder_length_min[joint] * CYLINDER_LENGTH_SCALE);
    return 0;
}

int Linearize_GetCylinderLengthMax(void *ctx, uint16_t *lmax)
{
    GETJOINT(joint);
    *(int16_t *)lmax = roundf(calibration_constants_stored.cylinder_length_max[joint] * CYLINDER_LENGTH_SCALE);
    return 0;
}

int Linearize_GetFeedbackLowpass(void *ctx, uint16_t *f)
{
    GETJOINT(joint);
    *f = round(10.0f * calibration_constants_stored.feedback_lowpass_frequency[joint]);
    return 0;
}

int Linearize_SetSensorVmin(void *ctx, uint16_t vmin)
{
    GETJOINT(joint);
    calibration_constants_stored.v_min[joint] = ((int16_t)vmin) / VOLTAGE_SCALE;
    Linearize_ConstantInit();
    return 0;
}

int Linearize_SetSensorVmax(void *ctx, uint16_t vmax)
{
    GETJOINT(joint);
    calibration_constants_stored.v_max[joint] = ((int16_t)vmax) / VOLTAGE_SCALE;
    Linearize_ConstantInit();
    return 0;
}

int Linearize_SetSensorThetamin(void *ctx, uint16_t tmin)
{
    GETJOINT(joint);
    calibration_constants_stored.theta_min[joint] = ((int16_t)tmin) / JOINT_ANGLE_SCALE;
    Linearize_ConstantInit();
    return 0;
}

int Linearize_SetSensorThetamax(void *ctx, uint16_t tmax)
{
    GETJOINT(joint);
    calibration_constants_stored.theta_max[joint] = ((int16_t)tmax) / JOINT_ANGLE_SCALE;
    Linearize_ConstantInit();
    return 0;
}

int Linearize_SetCylinderLengthMin(void *ctx, uint16_t lmin)
{
    GETJOINT(joint);
    calibration_constants_stored.cylinder_length_min[joint] = ((int16_t)lmin) / CYLINDER_LENGTH_SCALE;
    return 0;
}

int Linearize_SetCylinderLengthMax(void *ctx, uint16_t lmax)
{
    GETJOINT(joint);
    calibration_constants_stored.cylinder_length_max[joint] = ((int16_t)lmax) / CYLINDER_LENGTH_SCALE;
    return 0;
}

int Linearize_SetFeedbackLowpass(void *ctx, uint16_t f)
{
    GETJOINT(joint);
    calibration_constants_stored.feedback_lowpass_frequency[joint] = (float)f / 10.0f;
    Linearize_ConstantInit();
    return 0;
}

int Linearize_ReadAngle(void *ctx, uint16_t *v)
{
    GETJOINT(joint);
    *(int16_t *)v = roundf(joint_angle[joint] * JOINT_ANGLE_SCALE);
    return 0;
}

int Linearize_ReadLength(void *ctx, uint16_t *v)
{
    GETJOINT(joint);
    *v = roundf(cylinder_edge_length[joint] * CYLINDER_LENGTH_SCALE);
    return 0;
}

int Linearize_ReadSensorVoltage(void *ctx, uint16_t *v)
{
    GETJOINT(joint);
    *v = round(sensor_voltage[joint] * JOINT_ANGLE_SCALE);
    return 0;
}

int Linearize_ReadFeedbackVoltage(void *ctx, uint16_t *v)
{
    GETJOINT(joint);
    *v = round(feedback_voltage[joint] * JOINT_ANGLE_SCALE);
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
                compute_led_brightness(sensor_voltage);
            }
        }
        else if(event.value.signals & ADC_CONV_COMPLETE)
        {
            DAC_IO_LDAC(false);
            lowpass_sensor_readings(channel_values, sensor_voltage);
            compute_joint_angles(sensor_voltage, joint_angle);
            Kinematics_CylinderEdgeLengths(joint_angle, cylinder_edge_length);
            Linearize_ScaleCylinders(cylinder_edge_length, cylinder_scaled_values);
            Linearize_ComputeFeedback(cylinder_scaled_values, feedback_voltage, feedback_code);
            sendjoint = 0;
            ads5724_SetVoltage(joint_dac_channel[sendjoint], feedback_code[sendjoint]);
            sendjoint++;
        }
    }
}

void lowpass_sensor_readings(const uint32_t channel_values[JOINT_COUNT],
                             float voltage[JOINT_COUNT])
{
    for(enum JointIndex joint=0;joint<JOINT_COUNT;joint++)
    {
        voltage[joint] *= 1.0f - calibration_constants_processed.feedback_lowpass_constant[joint];
        voltage[joint] += calibration_constants_processed.feedback_lowpass_constant[joint] *
                          (ADC_VREF * (channel_values[joint_adc_channel[joint]] / ADC_MAX_CODE) / JOINT_DIVIDER);
    }
}

void compute_joint_angles(float voltage[JOINT_COUNT],
                          float joint_angle[JOINT_COUNT])
{
    for(enum JointIndex joint=0;joint<JOINT_COUNT;joint++)
    {
        joint_angle[joint] = (calibration_constants_processed.theta_offset[joint] +
                calibration_constants_processed.theta_scale[joint] * voltage[joint]);
    }
}

void Linearize_ScaleCylinders(const float cylinder_edge_length[JOINT_COUNT],
                              float scaled_values[JOINT_COUNT])
{
    for(int joint = 0; joint < JOINT_COUNT; joint++)
    {
        scaled_values[joint] = calibration_constants_processed.cylinder_scale[joint]*cylinder_edge_length[joint] +
                               calibration_constants_processed.cylinder_offset[joint];
    }
}

void Linearize_ScaleCylindersUnit(const float cylinder_edge_length[JOINT_COUNT],
                                  float scaled_values[JOINT_COUNT])
{
    for(int joint = 0; joint < JOINT_COUNT; joint++)
    {
        scaled_values[joint] = calibration_constants_processed.cylinder_scale[joint]*cylinder_edge_length[joint] + calibration_constants_processed.cylinder_offset[joint];
        scaled_values[joint] /= MAX_ENFIELD_SCALE;
    }
}


void Linearize_ComputeFeedback(const float scaled_values[JOINT_COUNT],
                               float feedback_voltage[JOINT_COUNT],
                               uint16_t feedback_code[JOINT_COUNT])
{
    for(int joint = 0; joint < JOINT_COUNT; joint++)
    {
        feedback_voltage[joint] = scaled_values[joint] * MAX_DAC_OUTPUT;
        feedback_code[joint] = scaled_values[joint] * DAC_MAX_CODE;
    }
}

void compute_led_brightness(const float joint_voltage[JOINT_COUNT])
{
    static int reduce_rate=LED_RATE_DIVISOR;
    if(--reduce_rate == 0)
    {
        float brightness;
        for(int joint=0;joint<JOINT_COUNT;joint++)
        {
            brightness = joint_voltage[joint] * JOINT_DIVIDER / ADC_VREF * 250.0f;
            brightness = (brightness > 255.0f) ? 255.0f : ((brightness < 0.0f) ? 0.0f : brightness);
            LED_SetOne(joint_led_channel[joint], 1, brightness);
        }
        reduce_rate = LED_RATE_DIVISOR;
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
