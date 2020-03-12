#pragma once

#include "joint.h"

#define CURL_BASE ((JOINT_CURL + 1) * 0x100)
#define SWING_BASE ((JOINT_SWING + 1) * 0x100)
#define LIFT_BASE ((JOINT_LIFT + 1) * 0x100)

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
    ICylinderLength = 0x12,
    IFeedbackVoltage = 0x13,
    ICachedBaseEndPressure =  0x20,
    ICachedRodEndPressure =  0x21,
    ISerialNumberLo = 0x30,
    ISerialNumberHi = 0x31,
    IAnalogCommand = 0x32,
    IFeedbackPosition = 0x33,
    IBaseEndPressure = 0x34,
    IRodEndPressure = 0x35,
    ISpoolPosition = 0x36,
    IFirmwareVersionB = 0x37,
    IFirmwareVersionA = 0x38,
    IFirmwareRevLow = 0x39,
    IFirmwareRevHigh = 0x40,
};

enum HoldingRegisterOffset {
    HProportionalGain = 0x20,
    HDerivativeGain = 0x21,
    HForceDamping = 0x22,
    HOffset = 0x23,
    HAreaRatio = 0x24,
    HCylinderBore = 0x25,
    HMinimumPosition = 0x26,
    HMaximumPosition = 0x27,
    HRampUp = 0x28,
    HRampDown = 0x29,
    HDeadBand = 0x30,
    HDitherAmplitude = 0x31,
    HValveOffset = 0x32,
    HDigitalCommand = 0x33,
    HCachedDigitalCommand = 0x34,
};
