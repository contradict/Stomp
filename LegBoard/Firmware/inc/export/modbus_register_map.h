#pragma once

#include "joint.h"

#define CURL_BASE ((JOINT_CURL + 1) * 0x100)
#define SWING_BASE ((JOINT_SWING + 1) * 0x100)
#define LIFT_BASE ((JOINT_LIFT + 1) * 0x100)

#define HMODBUSAddress 0x10
#define CSaveConstants 0x10

enum CoilOffset {
    CFeedbackPolarity = 0x20,
    CPortConnection = 0x21,
    CCommandInput = 0x22,
    CFeedbackInput = 0x23,
    CCommandSource = 0x24,
    CZeroGain = 0x25,
    CSaveConfiguration = 0x26,
};

enum InputRegisterOffset {
    ISensorVoltage = 0x10,
    IJointAngle = 0x11,
    IFeedbackVoltage = 0x12,
    ICylinderLength = 0x13,
    ICachedBaseEndPressure = 0x14,
    ICachedRodEndPressure = 0x15,
    ICachedFeedbackPosition = 0x16,
    ISerialNumberLo = 0x30,
    ISerialNumberHi = 0x31,
    IAnalogCommand = 0x32,
    IFeedbackPosition = 0x33,
    IBaseEndPressure = 0x34,
    IRodEndPressure = 0x35,
    ISpoolPosition = 0x36,
    IFirmwareVersionB = 0x37,
    IFirmwareVersionA = 0x3a,
    IFirmwareRevLow = 0x3b,
    IFirmwareRevHigh = 0x3c,
};

enum HoldingRegisterOffset {
    HSensorVmin = 0x10,
    HSensorVmax = 0x11,
    HSensorThetamin = 0x12,
    HSensorThetamax = 0x13,
    HCylinderLengthMin = 0x14,
    HCylinderLengthMax = 0x15,
    HProportionalGain = 0x30,
    HDerivativeGain = 0x31,
    HForceDamping = 0x32,
    HOffset = 0x33,
    HAreaRatio = 0x34,
    HCylinderBore = 0x35,
    HMinimumPosition = 0x36,
    HMaximumPosition = 0x37,
    HRampUp = 0x38,
    HRampDown = 0x39,
    HDeadBand = 0x3a,
    HDitherAmplitude = 0x3b,
    HValveOffset = 0x3c,
    HDigitalCommand = 0x3d,
    HCachedDigitalCommand = 0x3e,
};

enum LegCommands {
    SetToeX = 0x40,
    SetToeY = 0x41,
    SetToeZ = 0x42, // Causes write to servos
};

