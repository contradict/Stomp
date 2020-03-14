#include <math.h>
#include <complex.h>
#include "kinematics.h"

// See Kinematics.md

struct Link {
    complex float pivot;
    float l1, l2;
};

struct GeometryConstants {
    struct Link links[3];
};

static struct GeometryConstants geometry_constants __attribute__ ((section (".storage.linearize"))) = {
    .links = {
        { //CURL
            .pivot = 4.287 - 0.44I,
            .l1 = 2.5,
            .l2 = 6.0,
        },
        { //SWING
            .pivot = 3.603 + 2.572I,
            .l1 = 3.90,
            .l2 = 0.0,
        },
        { //LIFT
            .pivot = 4.287 + 2.06I,
            .l1 = 4.4,
            .l2 = 6.3,
        }
    }
};

void Kinematics_cylinder_edge_lengths(const float joint_angle[JOINT_COUNT],
                                   float cylinder_edge_length[JOINT_COUNT])
{
    cylinder_edge_length[JOINT_LIFT] = cabsf(
            geometry_constants.links[JOINT_LIFT].pivot -
            geometry_constants.links[JOINT_LIFT].l1 * cexpf(I * joint_angle[JOINT_LIFT]));
    cylinder_edge_length[JOINT_CURL] = cabsf(
            geometry_constants.links[JOINT_LIFT].pivot -
            geometry_constants.links[JOINT_CURL].pivot +
            geometry_constants.links[JOINT_CURL].l1 * cexpf(I * (joint_angle[JOINT_CURL] + joint_angle[JOINT_LIFT])) +
            geometry_constants.links[JOINT_LIFT].l2 * cexpf(I * joint_angle[JOINT_LIFT]));
    cylinder_edge_length[JOINT_SWING] = cabsf(
            geometry_constants.links[JOINT_SWING].pivot -
            geometry_constants.links[JOINT_SWING].l1 * cexpf(I * joint_angle[JOINT_SWING]));
}

void Kinematics_joint_angles(float toe[3], float joint_angle[JOINT_COUNT])
{
    complex float T = hypotf(toe[0], toe[1]) + I * toe[2];

    joint_angle[JOINT_CURL] = acosf(
        (cabsf(T - geometry_constants.links[JOINT_LIFT].pivot) - 
         (powf(geometry_constants.links[JOINT_LIFT].l2, 2.0f) + 
          powf(geometry_constants.links[JOINT_CURL].l2, 2.0f))) / 
        (2.0f * geometry_constants.links[JOINT_LIFT].l2 * geometry_constants.links[JOINT_CURL].l2));

    joint_angle[JOINT_LIFT] = asin(
        cimagf((T - geometry_constants.links[JOINT_LIFT].pivot) /
               (geometry_constants.links[JOINT_LIFT].l2 +
                geometry_constants.links[JOINT_CURL].l2 * cexpf(I * joint_angle[JOINT_CURL]))));

    joint_angle[JOINT_SWING] = atan2f(toe[1], toe[0]);
}
