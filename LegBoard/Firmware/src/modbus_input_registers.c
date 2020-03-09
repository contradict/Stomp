#include <modbus.h>

#include "modbus_common.h"

#include "linearize_feedback.h"
#include "enfield.h"

static uint16_t scratchpad = 0x55;

static int return_context(void *, uint16_t *v);

const struct MODBUS_InputRegister modbus_input_registers[] = {
    {
        .address = 0x55,
        .context = (void *)&scratchpad,
        .read = return_context
    },
    {
        .address = CURL_BASE,
        .context = (void *)JOINT_CURL,
        .read = Linearize_ReadAngle,
    },
    {
        .address = CURL_BASE + 1,
        .context = (void *)JOINT_CURL,
        .read = Linearize_ReadLength,
    },
    {
        .address = CURL_BASE + 2,
        .context = (void *)JOINT_CURL,
        .read = Enfield_ReadBaseEndPresure,
    },
    {
        .address = CURL_BASE + 3,
        .context = (void *)JOINT_CURL,
        .read = Enfield_ReadRodEndPresure,
    },
    {
        .address = SWING_BASE,
        .context = (void *)JOINT_SWING,
        .read = Linearize_ReadAngle,
    },
    {
        .address = SWING_BASE + 1,
        .context = (void *)JOINT_SWING,
        .read = Linearize_ReadLength,
    },
    {
        .address = SWING_BASE + 2,
        .context = (void *)JOINT_SWING,
        .read = Enfield_ReadBaseEndPresure,
    },
    {
        .address = SWING_BASE + 3,
        .context = (void *)JOINT_SWING,
        .read = Enfield_ReadRodEndPresure,
    },
    {
        .address = LIFT_BASE,
        .context = (void *)JOINT_LIFT,
        .read = Linearize_ReadAngle,
    },
    {
        .address = LIFT_BASE + 1,
        .context = (void *)JOINT_LIFT,
        .read = Linearize_ReadLength,
    },
    {
        .address = LIFT_BASE + 2,
        .context = (void *)JOINT_LIFT,
        .read = Enfield_ReadBaseEndPresure,
    },
    {
        .address = LIFT_BASE + 3,
        .context = (void *)JOINT_LIFT,
        .read = Enfield_ReadRodEndPresure,
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
    }
};

static int return_context(void *ctx, uint16_t *v)
{
    *v = *(bool *)ctx;
    return 0;
}
