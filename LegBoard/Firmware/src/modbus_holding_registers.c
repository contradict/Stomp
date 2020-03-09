#include <modbus.h>
#include "cmsis_os.h"

#include "modbus_common.h"

#include "enfield.h"

static uint16_t scratchpad = 0x55;

static const uint8_t EnfieldHoldingRegister[] = {
};

static int return_context(void *, uint16_t *v);
static int save_to_context(void *, uint16_t value);

struct MODBUS_HoldingRegister modbus_holding_registers[] = {
    {
        .address = 0x55,
        .context = (void *)&scratchpad,
        .read = return_context,
        .write = save_to_context
    },
    {
        .address = CURL_BASE,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetProportionalGain, ReadProportionalGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 1,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDerivativeGain,  ReadDerivativeGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 2,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetForceDamping,    ReadForceDamping),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 3,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetOffset,          ReadOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetAreaRatio,       ReadAreaRatio),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetCylinderBore,    ReadCylinderBore),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMinimumPosition, ReadMinimumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 7,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMaximumPosition, ReadMaximumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 8,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampUp,          ReadRampUp),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 9,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampDown,        ReadRampDown),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 10,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDeadBand,        ReadDeadBand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 11,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDitherAmplitude, ReadDitherAmplitude),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 12,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetValveOffset,     ReadValveOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + 13,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDigitalCommand,  ReadDigitalCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE,
        .context = (void *)ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetProportionalGain, ReadProportionalGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 1,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDerivativeGain,  ReadDerivativeGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 2,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetForceDamping,    ReadForceDamping),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 3,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetOffset,          ReadOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetAreaRatio,       ReadAreaRatio),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetCylinderBore,    ReadCylinderBore),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMinimumPosition, ReadMinimumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 7,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMaximumPosition, ReadMaximumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 8,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampUp,          ReadRampUp),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 9,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampDown,        ReadRampDown),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 10,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDeadBand,        ReadDeadBand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 11,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDitherAmplitude, ReadDitherAmplitude),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 12,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetValveOffset,     ReadValveOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + 13,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDigitalCommand,  ReadDigitalCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE,
        .context = (void *)ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetProportionalGain, ReadProportionalGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 1,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDerivativeGain,  ReadDerivativeGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 2,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetForceDamping,    ReadForceDamping),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 3,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetOffset,          ReadOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetAreaRatio,       ReadAreaRatio),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetCylinderBore,    ReadCylinderBore),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMinimumPosition, ReadMinimumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 7,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMaximumPosition, ReadMaximumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 8,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampUp,          ReadRampUp),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 9,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampDown,        ReadRampDown),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 10,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDeadBand,        ReadDeadBand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 11,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDitherAmplitude, ReadDitherAmplitude),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 12,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetValveOffset,     ReadValveOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + 13,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDigitalCommand,  ReadDigitalCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
        .write = 0
    }
};

static int return_context(void *ctx, uint16_t *v)
{
    *v = *(uint16_t *)ctx;
    return 0;
}

static int save_to_context(void *ctx, uint16_t value)
{
    *(uint16_t *)ctx = value;
    return 0;
}
