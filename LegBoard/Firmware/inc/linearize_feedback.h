#pragma once
#include <stdint.h>
#include "modbus.h"
#include "export/joint.h"

void Linearize_ThreadInit(void);
int Linearize_ReadAngle(void *ctx, uint16_t *v);
int Linearize_ReadLength(void *ctx, uint16_t *v);
int Linearize_ReadSensorVoltage(void *ctx, uint16_t *v);
int Linearize_ReadFeedbackVoltage(void *ctx, uint16_t *v);
void Linearize_GetJointAngles(float a[3]);
void Linearize_ScaleCylinders(const float cylinder_edge_length[JOINT_COUNT],
                              float scaled_values[JOINT_COUNT]);
