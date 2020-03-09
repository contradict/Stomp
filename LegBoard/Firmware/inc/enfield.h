#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"
#include "joint.h"

enum EnfieldReadRegister {
    ReadProportionalGain = 112,
    ReadDerivativeGain   = 113,
    ReadForceDamping     = 119,
    ReadOffset           = 126,
    ReadCommandInput     = 127,
    ReadFeedbackInput    = 128,
    ReadAreaRatio        = 129,
    ReadCylinderBore     = 130,
    ReadMinimumPosition  = 131,
    ReadMaximumPosition  = 132,
    ReadPortConnection   = 133,
    ReadRampUp           = 134,
    ReadRampDown         = 135,
    ReadDeadBand         = 137,
    ReadDitherAmplitude  = 138,
    ReadValveOffset      = 139,
    ReadFeedbackPolarity = 144,
    ReadDigitalCommand   = 145,
    ReadCommandSource    = 146,
    ReadSerialNumberLo   = 147,
    ReadSerialNumberHi   = 148,
    ReadAnalogCommand    = 152,
    ReadFeedbackPosition = 153,
    ReadBaseEndPressure  = 154,
    ReadRodEndPressure   = 155,
    ReadSpoolPosition    = 158,
    ReadFirmwareVersionB = 161,
    ReadFirmwareVersionA = 162,
    ReadFirmwareRevLow   = 163,
    ReadFirmwareRevHigh  = 164
};

enum EnfieldWriteRegister {
    SetProportionalGain  = 1,
    SetDerivativeGain    = 2,
    SetForceDamping      = 8,
    SetOffset            = 15,
    SetCommandInput      = 16,
    SetFeedbackInput     = 17,
    SetAreaRatio         = 18,
    SetCylinderBore      = 19,
    SetMinimumPosition   = 20,
    SetMaximumPosition   = 21,
    SetPortConnection    = 22,
    SetRampUp            = 23,
    SetRampDown          = 24,
    SetDeadBand          = 26,
    SetDitherAmplitude   = 27,
    SetValveOffset       = 28,
    SetFeedbackPolarity  = 33,
    SetCommandSource     = 89,
    SetDigitalCommand    = 88,
    SetZeroGains         = 224,
    SetSaveConfiguration = 225
};

#define COMMAND_SOURCE_DIGITAL 0
#define COMMAND_SOURCE_ANALOG  1

struct EnfieldResponse {
    int err;
    uint16_t value;
};

struct EnfieldRequest {
    enum JointIndex joint;
    bool write;
    enum EnfieldReadRegister r;
    enum EnfieldWriteRegister w;
    uint16_t value;
    osMailQId responseQ;
    struct EnfieldResponse *response;
};

void Enfield_Init();
struct EnfieldRequest * Enfield_AllocRequest(enum JointIndex joint);
void Enfield_Request(struct EnfieldRequest *req);
uint16_t Enfield_ReadRodEndPresure(void *ctx);
uint16_t Enfield_ReadBaseEndPresure(void *ctx);
