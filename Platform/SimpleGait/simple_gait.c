/*
 *
 * Leg Phase offset table maps leg index->phase offset
 *
 * Leg Phase is global phase + leg offset
 *
 * Step Shape Table maps leg phase to toe position
 *
 * Gait state machine
 *   Connect to all legs
 *   for ecah leg
 *     set command to measured for all joints
 *     ramp gain over ~1 sec
 *   Interpolate each leg to zero phase position over ~1 sec
 *   Accelerate to drive frequency over 1 sec
 *   execute 10 cycles
 *   decelrate to stop
 *   ramp gain to zero
 *   exit
*/

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <modbus/modbus.h>
#include "../../LegBoard/Firmware/inc/export/modbus_register_map.h"
#include "realtimer.h"

const float GainRampTime=1.0f;     // seconds
const float ProportionalGains[3] = {15.0f, 12.0f, 12.0f}; 
const float ExitLowGains[3] = {3.0f, 3.0f, 3.0f}; 
const float InitialPositionRampTime=2.0f; // seconds
const int NUM_WORKING_LEGS = 1;

enum LegIdentity {
    FrontLeft=0,
    CenterLeft=1,
    BackLeft=2,
    BackRight=3,
    CenterRight=4,
    FrontRight=5,
    NUM_LEGS
};

static const uint8_t LegAddress[NUM_LEGS] = {0x10, 0x20, 0x30, 0x40, 0x50, 0x60};
static const float SimpleGaitPhase[NUM_LEGS] = {0.0f, M_PI, 0.0f, M_PI, 0.0f, M_PI};

#define SimpleStepPoints 4
static const float StepShape[3][SimpleStepPoints] = {
// Forward down Backward down Backward up Forward up
/* X */ {10.1,       10.1,        10.1,      10.1},
/* Y */ { 4.0,       -4.0,        -4.0,       4.0},
/* Z */ {-7.3,       -7.3,        -4.7,      -4.7},
};
static const float StepShapePhase[SimpleStepPoints + 1] = {0.0f, 0.6, 0.7, 0.9, 1.0};


int find_interpolation_index(const float *nodes, size_t length, float x)
{
    size_t i;
    for(i=0; i<length-1; i++)
    {
        if((nodes[i] <= x) && (nodes[i+1] >= x))
            break;
    }
    if(i==length-1)
        return -1;
    return i;
}

void interpolate_value(const float *nodes, float *values, ssize_t length, int index, float x, float *y)
{
    int next_node = index+1;
    int next_value = next_node==length ? 0 : next_node;
    *y = (x - nodes[index]) * (values[next_value] - values[index]) / (nodes[next_node] - nodes[index]) + values[index];
}

int interpolate_toe_position(const float *nodes, float (*shape[])[3], size_t length, float phase, float (*toe_position)[3])
{
    int index = find_interpolation_index(nodes, length, phase);
    if(index<0)
        return index;
    for(int i=0;i<3;i++)
    {
        interpolate_value(nodes, (*shape)[i], length, index, phase, &((*toe_position)[i]));
    }
    return 0;
}

int set_joint_angles(modbus_t *ctx, enum LegIdentity leg, float (*toe_position)[3])
{
    uint16_t position_values[3];
    for(int i=0;i<3;i++)
    {
        position_values[i] = roundf(1000.0f * (*toe_position)[i]);
    }
    modbus_set_slave(ctx, LegAddress[leg]);
    return modbus_write_registers(ctx, ToeXPosition, 3, position_values);
}

int set_servo_gains(modbus_t *ctx, enum LegIdentity leg, const float (*gain)[3])
{
    int err;
    uint16_t gain_value;
    modbus_set_slave(ctx, LegAddress[leg]);
    gain_value = 1000.0f * (*gain)[JOINT_CURL];
    err = modbus_write_registers(ctx, CURL_BASE + HProportionalGain, 1, &gain_value);
    gain_value = 1000.0f * (*gain)[JOINT_SWING];
    if(err != -1)
        err = modbus_write_registers(ctx, SWING_BASE + HProportionalGain, 1, &gain_value);
    gain_value = 1000.0f * (*gain)[JOINT_LIFT];
    if(err != -1)
        err = modbus_write_registers(ctx, LIFT_BASE + HProportionalGain, 1, &gain_value);
    return err;
}

int set_initial_position(modbus_t *ctx, enum LegIdentity leg)
{
    int err;
    uint16_t command;
    modbus_set_slave(ctx, LegAddress[leg]);
    err = modbus_read_registers(ctx, CURL_BASE + IFeedbackPosition, 1, &command);
    if(err != -1)
        err = modbus_write_registers(ctx, CURL_BASE + HDigitalCommand, 1, &command);
    if(err != -1)
        err = modbus_read_registers(ctx, SWING_BASE + IFeedbackPosition, 1, &command);
    if(err != -1)
        err = modbus_write_registers(ctx, SWING_BASE + HDigitalCommand, 1, &command);
    if(err != -1)
        err = modbus_read_registers(ctx, LIFT_BASE + IFeedbackPosition, 1, &command);
    if(err != -1)
        err = modbus_write_registers(ctx, LIFT_BASE + HDigitalCommand, 1, &command);
    return err;
}

int get_toe_feedback(modbus_t *ctx, enum LegIdentity leg, float (*toe_position)[3], float (*cylinder_pressure)[6])
{
    int err;
    uint16_t toe_value[3];
    uint16_t cylinder_value[6];
    modbus_set_slave(ctx, LegAddress[leg]);
    err = modbus_read_registers(ctx, ToeXPosition, 3, toe_value);
    if(err != -1)
        for(int i=0;i<3;i++)
            (*toe_position)[i] = toe_value[i] / 1000.0f;
    if(err != -1)
        err = modbus_read_registers(ctx, CURL_BASE + ICachedBaseEndPressure, 2, &(cylinder_value[4]));
    if(err != -1)
        err = modbus_read_registers(ctx, SWING_BASE + ICachedBaseEndPressure, 2, &(cylinder_value[0]));
    if(err != -1)
        err = modbus_read_registers(ctx, LIFT_BASE + ICachedBaseEndPressure, 2, &(cylinder_value[2]));
    if(err != -1)
        for(int i=0;i<6;i++)
        {
            (*cylinder_pressure)[i] = cylinder_value[i] / 100.0f;
        }
    return err;
}

int set_toe_postion(modbus_t *ctx, enum LegIdentity leg, float (*toe_position)[3])
{
    uint16_t toe_values[3];

    for(int axis=0; axis < 3; axis++)
    {
        ((int16_t *)toe_values)[axis] = roundf((*toe_position)[axis] * 1000.0f);
    }
    modbus_set_slave(ctx, LegAddress[leg]);
    return modbus_write_registers(ctx, ToeXPosition, 3, toe_values);
}

int compute_leg_position(enum LegIdentity leg, float phase, float (*toe_position)[3])
{
    float discard;
    float leg_phase = modff(phase + SimpleGaitPhase[leg], &discard);
    return interpolate_toe_position(StepShapePhase, (float (**)[3])&StepShape, SimpleStepPoints, leg_phase, toe_position);
}

void walk(modbus_t *ctx, float period)
{
    /* Ping all the legs */
    for(int leg=0; leg<NUM_WORKING_LEGS; leg++)
    {
        int err = modbus_set_slave(ctx, LegAddress[leg]);
        uint16_t dummy;
        err = modbus_read_registers(ctx, 0x55, 1, &dummy);
        if(err == -1)
        {
            printf("Unable to communicate with leg %d(0x%04x): %s\n",
                    leg, LegAddress[leg], modbus_strerror(errno));
        }
        return;
    }

    printf("Ping all legs.\n");

    for(int leg=0; leg<NUM_WORKING_LEGS; leg++)
    {
        int err =set_initial_position(ctx, leg);
        if(err == -1)
        {
            printf("Failed to set initial position for leg %d(0x%04x): %s.",
                   leg, LegAddress[leg], modbus_strerror(errno));
            return;
        }
    }

    printf("Set inital command.\n");

    struct RealTimer tau;
    float elapsed;
    for(start_time(&tau); (elapsed = elapsed_time(&tau)) < GainRampTime; sleep_period(&tau, period))
    {
        float phase = elapsed / GainRampTime;
        float current_gain[3];
        for(int joint=0;joint<JOINT_COUNT;joint++)
        {
            current_gain[joint] = ProportionalGains[joint] * phase;
        }
        for(int leg=0; leg<NUM_WORKING_LEGS; leg++)
        {
            int err = set_servo_gains(ctx, leg, &current_gain);
            if(err == -1)
            {
                printf("Failed to set servo gain for leg %d(0x%04x): %s.",
                       leg, LegAddress[leg], modbus_strerror(errno));
                return;
            }
        }
    }

    printf("Servo gain ramp complete.\n");

    float startup_toe_positions[NUM_WORKING_LEGS][3];
    float initial_toe_positions[NUM_WORKING_LEGS][3];
    float discard_pressures[6];
    for(int leg=0; leg<NUM_WORKING_LEGS; leg++)
    {
        get_toe_feedback(ctx, leg, &(startup_toe_positions[leg]), &discard_pressures);
        compute_leg_position(leg, 0.0f, &(initial_toe_positions[leg]));
    }

    for(start_time(&tau); (elapsed = elapsed_time(&tau)) < InitialPositionRampTime; sleep_period(&tau, period))
    {
        float ramp_phase = elapsed / InitialPositionRampTime;
        for(int leg=0; leg<NUM_WORKING_LEGS; leg++)
        {
            float ramp_position[3];
            for(int axis=0;axis<3;axis++)
            {
                ramp_position[axis] = ramp_phase * initial_toe_positions[leg][axis] + (1.0 - ramp_phase) * startup_toe_positions[leg][axis];
                int err = set_toe_postion(ctx, leg, &ramp_position);
                if(err == -1)
                {
                    printf("Failed to ramp initial position for leg %d(0x%04x): %s",
                           leg, LegAddress[leg], modbus_strerror(errno));
                    goto lowgainexit;
                }
            }
        }
    }

    printf("Leg position ramp complete.\n");

lowgainexit:
    for(int leg=0; leg<NUM_WORKING_LEGS; leg++)
    {
        int err = set_servo_gains(ctx, leg, &ExitLowGains);
        if(err == -1)
        {
            printf("Failed to set servo gain low for leg %d(0x%04x): %s.",
                   leg, LegAddress[leg], modbus_strerror(errno));
        }
    }
}
