#include <modbus.h>

#include "modbus_common.h"

#include "export/modbus_register_map.h"

#include "linearize_feedback.h"
#include "enfield.h"

static uint16_t scratchpad = 0x55;

const struct MODBUS_InputRegister modbus_input_registers[] = {
    {
        .address = 0x55,
        .context = (void *)&scratchpad,
        .read = return_context
    },
    {
        .address = CURL_BASE + ISensorVoltage,
        .context = (void *)JOINT_CURL,
        .read = Linearize_ReadSensorVoltage,
    },
    {
        .address = CURL_BASE + IJointAngle,
        .context = (void *)JOINT_CURL,
        .read = Linearize_ReadAngle,
    },
    {
        .address = CURL_BASE + ICylinderLength,
        .context = (void *)JOINT_CURL,
        .read = Linearize_ReadLength,
    },
    {
        .address = CURL_BASE + IFeedbackVoltage,
        .context = (void *)JOINT_CURL,
        .read = Linearize_ReadFeedbackVoltage,
    },
    {
        .address = CURL_BASE + ICachedBaseEndPressure,
        .context = (void *)JOINT_CURL,
        .read = Enfield_ReadBaseEndPresure,
    },
    {
        .address = CURL_BASE + ICachedRodEndPressure,
        .context = (void *)JOINT_CURL,
        .read = Enfield_ReadRodEndPresure,
    },
    {
        .address = CURL_BASE + ICachedFeedbackPosition,
        .context = (void *)JOINT_CURL,
        .read = Enfield_ReadFeedbackPosition,
    },
    {
        .address = CURL_BASE + ISerialNumberLo,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadSerialNumberLo),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + ISerialNumberHi,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadSerialNumberHi),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IAnalogCommand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadAnalogCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IFeedbackPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFeedbackPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IBaseEndPressure,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadBaseEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IRodEndPressure,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadRodEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + ISpoolPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadSpoolPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IFirmwareVersionB,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareVersionB),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IFirmwareVersionA,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareVersionA),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IFirmwareRevLow,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareRevLow),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = CURL_BASE + IFirmwareRevHigh,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1, ReadFirmwareRevHigh),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + ISensorVoltage,
        .context = (void *)JOINT_SWING,
        .read = Linearize_ReadSensorVoltage,
    },
    {
        .address = SWING_BASE + IJointAngle,
        .context = (void *)JOINT_SWING,
        .read = Linearize_ReadAngle,
    },
    {
        .address = SWING_BASE + ICylinderLength,
        .context = (void *)JOINT_SWING,
        .read = Linearize_ReadLength,
    },
    {
        .address = SWING_BASE + IFeedbackVoltage,
        .context = (void *)JOINT_SWING,
        .read = Linearize_ReadFeedbackVoltage,
    },
    {
        .address = SWING_BASE + ICachedBaseEndPressure,
        .context = (void *)JOINT_SWING,
        .read = Enfield_ReadBaseEndPresure,
    },
    {
        .address = SWING_BASE + ICachedRodEndPressure,
        .context = (void *)JOINT_SWING,
        .read = Enfield_ReadRodEndPresure,
    },
    {
        .address = SWING_BASE + ICachedFeedbackPosition,
        .context = (void *)JOINT_SWING,
        .read = Enfield_ReadFeedbackPosition,
    },
    {
        .address = SWING_BASE + ISerialNumberLo,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadSerialNumberLo),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + ISerialNumberHi,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadSerialNumberHi),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IAnalogCommand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadAnalogCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IFeedbackPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFeedbackPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IBaseEndPressure,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadBaseEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IRodEndPressure,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadRodEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + ISpoolPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadSpoolPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IFirmwareVersionB,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareVersionB),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IFirmwareVersionA,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareVersionA),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IFirmwareRevLow,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareRevLow),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = SWING_BASE + IFirmwareRevHigh,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1, ReadFirmwareRevHigh),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + ISensorVoltage,
        .context = (void *)JOINT_LIFT,
        .read = Linearize_ReadSensorVoltage,
    },
    {
        .address = LIFT_BASE + IJointAngle,
        .context = (void *)JOINT_LIFT,
        .read = Linearize_ReadAngle,
    },
    {
        .address = LIFT_BASE + ICylinderLength,
        .context = (void *)JOINT_LIFT,
        .read = Linearize_ReadLength,
    },
    {
        .address = LIFT_BASE + IFeedbackVoltage,
        .context = (void *)JOINT_LIFT,
        .read = Linearize_ReadFeedbackVoltage,
    },
    {
        .address = LIFT_BASE + ICachedBaseEndPressure,
        .context = (void *)JOINT_LIFT,
        .read = Enfield_ReadBaseEndPresure,
    },
    {
        .address = LIFT_BASE + ICachedRodEndPressure,
        .context = (void *)JOINT_LIFT,
        .read = Enfield_ReadRodEndPresure,
    },
    {
        .address = LIFT_BASE + ICachedFeedbackPosition,
        .context = (void *)JOINT_LIFT,
        .read = Enfield_ReadFeedbackPosition,
    },
    {
        .address = LIFT_BASE + ISerialNumberLo,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadSerialNumberLo),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + ISerialNumberHi,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadSerialNumberHi),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IAnalogCommand,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadAnalogCommand),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IFeedbackPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFeedbackPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IBaseEndPressure,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadBaseEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IRodEndPressure,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadRodEndPressure),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + ISpoolPosition,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadSpoolPosition),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IFirmwareVersionB,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareVersionB),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IFirmwareVersionA,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareVersionA),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IFirmwareRevLow,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareRevLow),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = LIFT_BASE + IFirmwareRevHigh,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1, ReadFirmwareRevHigh),
        .read = MODBUS_ReadEnfieldHoldingRegister
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
    }
};
