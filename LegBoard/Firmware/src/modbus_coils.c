#include "modbus.h"
#include "modbus_common.h"

#include "enfield.h"

static uint16_t scratchpad = 0x55;

static int return_context(void *ctx, bool *v);
static int save_to_context(void *ctx, bool value);

const struct MODBUS_Coil modbus_coils[] = {
    {
        .address = 0x55,
        .context = &scratchpad,
        .read = return_context,
        .write = save_to_context
    },
    {
        .address = CURL_BASE,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadFeedbackPolarity, SetFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + 1,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadPortConnection,   SetPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + 2,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadCommandInput,     SetCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + 3,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadFeedbackInput,    SetFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, ReadCommandSource,    SetCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1,                   SetZeroGains),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = CURL_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_CURL, -1,                   SetSaveConfiguration),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadFeedbackPolarity, SetFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + 1,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadPortConnection,   SetPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + 2,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadCommandInput,     SetCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + 3,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadFeedbackInput,    SetFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, ReadCommandSource,    SetCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1,                   SetZeroGains),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = SWING_BASE + 6,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_SWING, -1,                   SetSaveConfiguration),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadFeedbackPolarity, SetFeedbackPolarity),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + 1,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadPortConnection,   SetPortConnection),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + 2,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadCommandInput,     SetCommandInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + 3,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadFeedbackInput,    SetFeedbackInput),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + 4,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, ReadCommandSource,    SetCommandSource),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + 5,
        .context = ENFIELD_CONTEXT_VALUE(JOINT_LIFT, -1,                   SetZeroGains),
        .read = MODBUS_ReadEnfieldCoil,
        .write = MODBUS_WriteEnfieldCoil
    },
    {
        .address = LIFT_BASE + 6,
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

static int return_context(void *ctx, bool *v)
{
    *v = *(bool *)ctx;
    return 0;
}

static int save_to_context(void *ctx, bool value)
{
    *(uint16_t *)ctx = value;
    return 0;
}
