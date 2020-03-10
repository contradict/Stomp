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
        .address = CURL_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadSerialNumberLo),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadSerialNumberHi),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadAnalogCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 7,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFeedbackPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 8,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadBaseEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 9,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadRodEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 10,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadSpoolPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 11,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareVersionB),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 12,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareVersionA),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 13,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareRevLow),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + 14,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareRevHigh),
        .read = MODBUS_ReadEnfieldHoldingRegister
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
        .address = SWING_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadSerialNumberLo),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadSerialNumberHi),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadAnalogCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 7,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFeedbackPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 8,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadBaseEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 9,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadRodEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 10,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadSpoolPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 11,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareVersionB),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 12,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareVersionA),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 13,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareRevLow),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + 14,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareRevHigh),
        .read = MODBUS_ReadEnfieldHoldingRegister
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
        .address = LIFT_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadSerialNumberLo),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadSerialNumberHi),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadAnalogCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 7,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFeedbackPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 8,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadBaseEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 9,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadRodEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 10,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadSpoolPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 11,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareVersionB),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 12,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareVersionA),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 13,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareRevLow),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + 14,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareRevHigh),
        .read = MODBUS_ReadEnfieldHoldingRegister
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
