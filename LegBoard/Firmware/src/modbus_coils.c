#include "modbus.h"
#include "export/modbus_register_map.h"
#include "modbus_common.h"

#include "enfield.h"

#define ENFIELD_CONTEXT_VALUE(j, w, r) ((void *)(((j&3)<<30) | ((w&0xff)<<8) | (r&0xff)))

static uint16_t scratchpad = 0x55;

const struct MODBUS_Coil modbus_coils[] = {
    {
        .address = 0x55,
        .context = &scratchpad,
        .read = return_context_bool,
        .write = save_to_context_bool
    },
    {
        .address = CURL_BASE + CFeedbackPolarity,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetFeedbackPolarity,  ReadFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CPortConnection,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetPortConnection,      ReadPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CCommandInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetCommandInput,          ReadCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CFeedbackInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetFeedbackInput,        ReadFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CCommandSource,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetCommandSource,        ReadCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CZeroGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetZeroGains,                           -1),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CSaveConfiguration,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, SetSaveConfiguration,                   -1),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CFeedbackPolarity,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetFeedbackPolarity, ReadFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CPortConnection,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetPortConnection,     ReadPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CCommandInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetCommandInput,         ReadCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CFeedbackInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetFeedbackInput,       ReadFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CCommandSource,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetCommandSource,       ReadCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CZeroGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetZeroGains,                          -1),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CSaveConfiguration,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, SetSaveConfiguration,                  -1),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CFeedbackPolarity,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetFeedbackPolarity,  ReadFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CPortConnection,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetPortConnection,      ReadPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CCommandInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetCommandInput,          ReadCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CFeedbackInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetFeedbackInput,        ReadFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CCommandSource,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetCommandSource,        ReadCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CZeroGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetZeroGains,                           -1),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CSaveConfiguration,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, SetSaveConfiguration,                   -1),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = 0,
        .context = 0,
        .read = 0,
        .write = 0
    }
};
