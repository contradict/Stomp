#include <modbus.h>

int ping_leg(modbus_t *ctx, uint8_t address);
int set_servo_gains(modbus_t *ctx, uint8_t address, const float (*pgain)[3], const float (*dgain)[3], const float (*damping)[3]);
int get_toe_feedback(modbus_t *ctx, uint8_t address, float (*toe_position)[3], float (*cylinder_pressure)[6]);
int set_toe_postion(modbus_t *ctx, uint8_t address, float (*toe_position)[3]);
