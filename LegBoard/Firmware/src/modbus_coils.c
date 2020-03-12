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
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadFeedbackPolarity, SetFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CPortConnection,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadPortConnection,   SetPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CCommandInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadCommandInput,     SetCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CFeedbackInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadFeedbackInput,    SetFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CCommandSource,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadCommandSource,    SetCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CZeroGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1,                   SetZeroGains),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + CSaveConfiguration,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1,                   SetSaveConfiguration),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CFeedbackPolarity,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadFeedbackPolarity, SetFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CPortConnection,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadPortConnection,   SetPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CCommandInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadCommandInput,     SetCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CFeedbackInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadFeedbackInput,    SetFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CCommandSource,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadCommandSource,    SetCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CZeroGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1,                   SetZeroGains),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + CSaveConfiguration,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1,                   SetSaveConfiguration),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CFeedbackPolarity,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadFeedbackPolarity, SetFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CPortConnection,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadPortConnection,   SetPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CCommandInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadCommandInput,     SetCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CFeedbackInput,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadFeedbackInput,    SetFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CCommandSource,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadCommandSource,    SetCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CZeroGain,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1,                   SetZeroGains),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + CSaveConfiguration,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1,                   SetSaveConfiguration),
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
