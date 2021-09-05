#include <errno.h>
#include <math.h>

#include "sclog4c/sclog4c.h"

#include "modbus_register_map.h"

#include "leg_control/modbus_utils.h"

#define PING_KEY 0xa956

int ping_leg(modbus_t *ctx, uint8_t address)
{
        uint16_t data=PING_KEY;
        int err;
        modbus_set_slave(ctx, address);
        err = modbus_diagnostics(ctx, MODBUS_DIAGNOSTICS_RETURN_QUERY_DATA, &data);
        if(err == -1)
            return err;
        return data == PING_KEY;
}

int set_servo_gains(modbus_t *ctx, uint8_t address, const float (*pgain)[3], const float (*dgain)[3], const float (*damping)[3], const float (*feedback_lowpass)[3])
{
    int ret=0, err;
    uint16_t pgain_value, dgain_value, damping_value, lp;
    modbus_set_slave(ctx, address);
    pgain_value = 10.0f * (*pgain)[JOINT_CURL];
    dgain_value = 10.0f * (*dgain)[JOINT_CURL];
    damping_value = 10.0f * (*damping)[JOINT_CURL];
    lp = 10.0f * (*feedback_lowpass)[JOINT_CURL];
    err = modbus_write_registers(ctx, CURL_BASE + HProportionalGain, 1, &pgain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Curl proportional gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, CURL_BASE + HDerivativeGain, 1, &dgain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Curl derivative gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, CURL_BASE + HForceDamping, 1, &damping_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Curl damping: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, CURL_BASE + HFeedbackLowpass, 1, &lp);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Curl feedback lowpass: %s", modbus_strerror(errno));
        ret = err;
    }

    pgain_value = 10.0f * (*pgain)[JOINT_SWING];
    dgain_value = 10.0f * (*dgain)[JOINT_SWING];
    damping_value = 10.0f * (*damping)[JOINT_SWING];
    lp = 10.0f * (*feedback_lowpass)[JOINT_SWING];
    err = modbus_write_registers(ctx, SWING_BASE + HProportionalGain, 1, &pgain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Swing proportional gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, SWING_BASE + HDerivativeGain, 1, &dgain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Swing derivative gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, SWING_BASE + HForceDamping, 1, &damping_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Swing damping: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, SWING_BASE + HFeedbackLowpass, 1, &lp);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Swing feedback lowpass: %s", modbus_strerror(errno));
        ret = err;
    }

    pgain_value = 10.0f * (*pgain)[JOINT_LIFT];
    dgain_value = 10.0f * (*dgain)[JOINT_LIFT];
    damping_value = 10.0f * (*damping)[JOINT_LIFT];
    lp = 10.0f * (*feedback_lowpass)[JOINT_LIFT];
    err = modbus_write_registers(ctx, LIFT_BASE + HProportionalGain, 1, &pgain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Lift proportional gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, LIFT_BASE + HDerivativeGain, 1, &dgain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Lift derivative gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, LIFT_BASE + HForceDamping, 1, &damping_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Lift damping: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, LIFT_BASE + HFeedbackLowpass, 1, &lp);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Lift feedback lowpass: %s", modbus_strerror(errno));
        ret = err;
    }

    return ret;
}

int get_toe_feedback(modbus_t *ctx, uint8_t address, float (*toe_position)[3], float (*cylinder_pressure)[6])
{
    int err;
    uint16_t toe_value[9];
    modbus_set_slave(ctx, address);
    err = modbus_read_input_registers(ctx, IFeedbackToePositionX, 9, toe_value);
    if(err != -1)
    {
        for(int i=0;i<3;i++)
            (*toe_position)[i] = ((int16_t *)toe_value)[i] / 1e4f;
        for(int i=3;i<9;i++)
            (*cylinder_pressure)[i-3] = toe_value[i] / 25.6f;
    }
    return err == -1 ? -1 : 0;
}

int set_toe_postion(modbus_t *ctx, uint8_t address, float (*toe_position)[3])
{
    uint16_t toe_values[3];

    for(int axis=0; axis < 3; axis++)
    {
        ((int16_t *)toe_values)[axis] = roundf((*toe_position)[axis] * 10000.0f);
    }
    logm(SL4C_FINE, "addr 0x%02x f:[%5.3f, %5.3f, %5.3f] s:[%4d, %4d, %4d] u:[%4x, %4x, %4x]",
         address,
         (*toe_position)[0], (*toe_position)[1], (*toe_position)[2],
         ((int16_t*)toe_values)[0], ((int16_t*)toe_values)[1], ((int16_t*)toe_values)[2],
         toe_values[0], toe_values[1], toe_values[2]);
    modbus_set_slave(ctx, address);
    return modbus_write_registers(ctx, ToeXPosition, 3, toe_values);
}
