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

int set_servo_gains(modbus_t *ctx, uint8_t address, const float (*gain)[3], const float (*damping)[3])
{
    int ret=0, err;
    uint16_t gain_value, damping_value;
    modbus_set_slave(ctx, address);
    gain_value = 10.0f * (*gain)[JOINT_CURL];
    damping_value = 10.0f * (*damping)[JOINT_CURL];
    err = modbus_write_registers(ctx, CURL_BASE + HProportionalGain, 1, &gain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Curl gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, CURL_BASE + HForceDamping, 1, &damping_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Curl damping: %s", modbus_strerror(errno));
        ret = err;
    }
    gain_value = 10.0f * (*gain)[JOINT_SWING];
    damping_value = 10.0f * (*damping)[JOINT_SWING];
    err = modbus_write_registers(ctx, SWING_BASE + HProportionalGain, 1, &gain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Swing gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, SWING_BASE + HForceDamping, 1, &damping_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Swing damping: %s", modbus_strerror(errno));
        ret = err;
    }
    gain_value = 10.0f * (*gain)[JOINT_LIFT];
    damping_value = 10.0f * (*damping)[JOINT_LIFT];
    err = modbus_write_registers(ctx, LIFT_BASE + HForceDamping, 1, &damping_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Lift gain: %s", modbus_strerror(errno));
        ret = err;
    }
    err = modbus_write_registers(ctx, LIFT_BASE + HProportionalGain, 1, &gain_value);
    if(err == -1)
    {
        logm(SL4C_ERROR, "Could not set Lift damping: %s", modbus_strerror(errno));
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
            (*cylinder_pressure)[i-3] = toe_value[i] / 100.0f;
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
