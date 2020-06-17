#include <modbus.h>
#include "cmsis_os.h"

#include "modbus_common.h"

#include "export/modbus_register_map.h"

#include "enfield.h"

#include "kinematics.h"

#include "linearize_feedback.h"

static uint16_t scratchpad = 0x55;

static const uint8_t EnfieldHoldingRegister[] = {
};

struct MODBUS_HoldingRegister modbus_holding_registers[] = {
    {
        .address = HMODBUSAddress,
        .context = (void *)0,
        .read = MODBUS_GetAddress,
        .write = MODBUS_SetAddress,
    },
    {
        .address = 0x40,
        .context = (void *)0,
        .read = Kinematics_ReadToePosition,
        .write = Kinematics_WriteToePosition,
    },
    {
        .address = 0x41,
        .context = (void *)1,
        .read = Kinematics_ReadToePosition,
        .write = Kinematics_WriteToePosition,
    },
    {
        .address = 0x42,
        .context = (void *)2,
        .read = Kinematics_ReadToePosition,
        .write = Kinematics_WriteToePosition,
    },
    {
        .address = 0x55,
        .context = (void *)&scratchpad,
        .read = return_context,
        .write = save_to_context
    },
    {
        .address = CURL_BASE + HSensorVmin,
        .context = (void *)JOINT_CURL,
        .read = Linearize_GetSensorVmin,
        .write = Linearize_SetSensorVmin,
    },
    {
        .address = CURL_BASE + HSensorVmax,
        .context = (void *)JOINT_CURL,
        .read = Linearize_GetSensorVmax,
        .write = Linearize_SetSensorVmax,
    },
    {
        .address = CURL_BASE + HSensorThetamin,
        .context = (void *)JOINT_CURL,
        .read = Linearize_GetSensorThetamin,
        .write = Linearize_SetSensorThetamin,
    },
    {
        .address = CURL_BASE + HSensorThetamax,
        .context = (void *)JOINT_CURL,
        .read = Linearize_GetSensorThetamax,
        .write = Linearize_SetSensorThetamax,
    },
    {
        .address = CURL_BASE + HCylinderLengthMin,
        .context = (void *)JOINT_CURL,
        .read = Linearize_GetCylinderLengthMin,
        .write = Linearize_SetCylinderLengthMin,
    },
    {
        .address = CURL_BASE + HCylinderLengthMax,
        .context = (void *)JOINT_CURL,
        .read = Linearize_GetCylinderLengthMax,
        .write = Linearize_SetCylinderLengthMax,
    },
    {
        .address = CURL_BASE + HProportionalGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetProportionalGain, ReadProportionalGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HDerivativeGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDerivativeGain,  ReadDerivativeGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HForceDamping,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetForceDamping,    ReadForceDamping),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HOffset,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetOffset,          ReadOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HAreaRatio,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetAreaRatio,       ReadAreaRatio),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HCylinderBore,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetCylinderBore,    ReadCylinderBore),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HMinimumPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMinimumPosition, ReadMinimumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HMaximumPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetMaximumPosition, ReadMaximumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HRampUp,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampUp,          ReadRampUp),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HRampDown,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetRampDown,        ReadRampDown),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HDeadBand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDeadBand,        ReadDeadBand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HDitherAmplitude,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDitherAmplitude, ReadDitherAmplitude),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HValveOffset,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetValveOffset,     ReadValveOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HDigitalCommand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetDigitalCommand,  ReadDigitalCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = CURL_BASE + HCachedDigitalCommand,
        .context = (void *)JOINT_CURL,
        .read = Enfield_ReadDigitalCommand,
        .write = Enfield_WriteDigitalCommand
    },
    {
        .address = SWING_BASE + HProportionalGain,
        .context = (void *)ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetProportionalGain, ReadProportionalGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HDerivativeGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetDerivativeGain,  ReadDerivativeGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HForceDamping,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetForceDamping,    ReadForceDamping),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HOffset,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetOffset,          ReadOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HAreaRatio,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetAreaRatio,       ReadAreaRatio),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HCylinderBore,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetCylinderBore,    ReadCylinderBore),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HMinimumPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetMinimumPosition, ReadMinimumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HMaximumPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetMaximumPosition, ReadMaximumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HRampUp,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetRampUp,          ReadRampUp),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HRampDown,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetRampDown,        ReadRampDown),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HDeadBand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetDeadBand,        ReadDeadBand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HDitherAmplitude,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetDitherAmplitude, ReadDitherAmplitude),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HValveOffset,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetValveOffset,     ReadValveOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HDigitalCommand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetDigitalCommand,  ReadDigitalCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = SWING_BASE + HCachedDigitalCommand,
        .context = (void *)JOINT_SWING,
        .read = Enfield_ReadDigitalCommand,
        .write = Enfield_WriteDigitalCommand
    },
    {
        .address = LIFT_BASE + HProportionalGain,
        .context = (void *)ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetProportionalGain, ReadProportionalGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HDerivativeGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetDerivativeGain,  ReadDerivativeGain),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HForceDamping,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetForceDamping,    ReadForceDamping),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HOffset,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetOffset,          ReadOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HAreaRatio,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetAreaRatio,       ReadAreaRatio),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HCylinderBore,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetCylinderBore,    ReadCylinderBore),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HMinimumPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetMinimumPosition, ReadMinimumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HMaximumPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetMaximumPosition, ReadMaximumPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HRampUp,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetRampUp,          ReadRampUp),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HRampDown,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetRampDown,        ReadRampDown),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HDeadBand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetDeadBand,        ReadDeadBand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HDitherAmplitude,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetDitherAmplitude, ReadDitherAmplitude),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HValveOffset,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetValveOffset,     ReadValveOffset),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HDigitalCommand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetDigitalCommand,  ReadDigitalCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister,
        .write = MODBUS_WriteEnfieldHoldingRegister,
    },
    {
        .address = LIFT_BASE + HCachedDigitalCommand,
        .context = (void *)JOINT_LIFT,
        .read = Enfield_ReadDigitalCommand,
        .write = Enfield_WriteDigitalCommand
    }, 
    {
        .address = 0,
        .context = 0,
        .read = 0,
        .write = 0
    }
};
