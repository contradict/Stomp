#pragma once
#include <stdint.h>
#include "export/joint.h"

void Kinematics_CylinderEdgeLengths(const float joint_angle[JOINT_COUNT],
                                      float cylinder_edge_length[JOINT_COUNT]);

void Kinematics_JointAngles(const float toe[3], float joint_angle[JOINT_COUNT]);

int Kinematics_ReadToePosition(void *context, int16_t *v);
int Kinematics_WriteToePosition(void *context, int16_t v);
