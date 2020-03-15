#include <math.h>
#include <complex.h>
#include "linearize_feedback.h"
#include "kinematics.h"
#include "modbus.h"
#include "enfield.h"

#define LENGTH_SCALE 1000.0f

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

static float commanded_position[3];

void Kinematics_CylinderEdgeLengths(const float joint_angle[JOINT_COUNT],
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

void Kinematics_JointAngles(const float toe[3], float joint_angle[JOINT_COUNT])
{
    complex float T = hypotf(toe[0], toe[1]) + I * toe[2];

    joint_angle[JOINT_CURL] = -acosf(
        (powf(cabsf(T - geometry_constants.links[JOINT_LIFT].pivot), 2.0f) - 
         (powf(geometry_constants.links[JOINT_LIFT].l2, 2.0f) + 
          powf(geometry_constants.links[JOINT_CURL].l2, 2.0f))) / 
        (2.0f * geometry_constants.links[JOINT_LIFT].l2 * geometry_constants.links[JOINT_CURL].l2));

    joint_angle[JOINT_LIFT] = asinf(
        cimagf((T - geometry_constants.links[JOINT_LIFT].pivot) /
               (geometry_constants.links[JOINT_LIFT].l2 +
                geometry_constants.links[JOINT_CURL].l2 * cexpf(I * joint_angle[JOINT_CURL]))));

    joint_angle[JOINT_SWING] = atan2f(toe[1], toe[0]);
}

void Kinematics_ToePosition(const float joint_angle[JOINT_COUNT],
                            float position[3])
{
    complex float T = geometry_constants.links[JOINT_LIFT].pivot +
        geometry_constants.links[JOINT_LIFT].l2 * cexpf(I * joint_angle[JOINT_LIFT]) +
        geometry_constants.links[JOINT_CURL].l2 * cexpf(I * (joint_angle[JOINT_LIFT] + joint_angle[JOINT_CURL]));
    float r = crealf(T);
    position[0] = cosf(joint_angle[JOINT_SWING]) * r;
    position[1] = sinf(joint_angle[JOINT_SWING]) * r;
    position[2] = cimagf(T);
}

int Kinematics_ReadCurrentEdgeLengths(void *ctx, uint16_t *v)
{
    int joint = (int)ctx;
    float joint_angle[3];
    float edge_length[3];

    if((joint < 0) || (joint > JOINT_COUNT))
        return ILLEGAL_DATA_ADDRESS;
    Linearize_GetJointAngles(joint_angle);
    Kinematics_CylinderEdgeLengths(joint_angle, edge_length);
    *v = roundf(LENGTH_SCALE * edge_length[joint]);
    return 0;
}

int Kinematics_ReadToePosition(void *ctx, int16_t *v)
{
    int coordinate = (int)ctx;
    float joint_angle[JOINT_COUNT];
    float toe_postion[3];

    if((coordinate < 0) || (coordinate > 2))
        return ILLEGAL_DATA_ADDRESS;
    Linearize_GetJointAngles(joint_angle);
    Kinematics_ToePosition(joint_angle, toe_postion);
    *v = roundf(LENGTH_SCALE * toe_postion[coordinate]);
    return 0;
}

int Kinematics_WriteToePosition(void *ctx, int16_t v)
{
    int coordinate = (int)ctx;
    float joint_angle[3];
    float cylinder_edge_length[3];
    float cylinder_scaled_value[3];
    uint16_t cylinder_command[3];

    if((coordinate < 0) || (coordinate > 2))
        return ILLEGAL_DATA_ADDRESS;
    commanded_position[coordinate] = v / LENGTH_SCALE;
    if(coordinate == 2)
    {
        Kinematics_JointAngles(commanded_position, joint_angle);
        Kinematics_CylinderEdgeLengths(joint_angle, cylinder_edge_length);
        Linearize_ScaleCylinders(cylinder_edge_length, cylinder_scaled_value);
        for(int j=0;j<JOINT_COUNT;j++)
            cylinder_command[j] = cylinder_scaled_value[j] * 4095;
        Enfield_SetCommand(cylinder_command);
    }
    return 0;
}
