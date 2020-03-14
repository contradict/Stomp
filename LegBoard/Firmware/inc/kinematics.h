#pragma once
#include "export/joint.h"

void Kinematics_cylinder_edge_lengths(const float joint_angle[JOINT_COUNT],
                                      float cylinder_edge_length[JOINT_COUNT]);

void Kinematics_joint_angles(float toe[3], float joint_angle[JOINT_COUNT]);
