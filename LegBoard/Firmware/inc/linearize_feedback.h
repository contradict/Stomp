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
                              float scaled_values[JOINT_COUNT],
                              float scalemax);
int Linearize_SetSensorVmin(void *ctx, uint16_t vmin);
int Linearize_GetSensorVmin(void *ctx, uint16_t *vmin);
int Linearize_SetSensorVmax(void *ctx, uint16_t vmax);
int Linearize_GetSensorVmax(void *ctx, uint16_t *vmax);
int Linearize_SetSensorThetamin(void *ctx, uint16_t tmin);
int Linearize_GetSensorThetamin(void *ctx, uint16_t *tmin);
int Linearize_SetSensorThetamax(void *ctx, uint16_t tmax);
int Linearize_GetSensorThetamax(void *ctx, uint16_t *tmax);
int Linearize_SetCylinderLengthMin(void *ctx, uint16_t lmin);
int Linearize_GetCylinderLengthMin(void *ctx, uint16_t *lmin);
int Linearize_SetCylinderLengthMax(void *ctx, uint16_t lmax);
int Linearize_GetCylinderLengthMax(void *ctx, uint16_t *lmax);
