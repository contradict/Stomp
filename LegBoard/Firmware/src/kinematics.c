#include <math.h>
#include <complex.h>
#include "linearize_feedback.h"
#include "kinematics.h"
#include "modbus.h"
#include "enfield.h"
#include <errno.h>


// See Kinematics.md

static const float JOINT_ANGLE_SCALE = 1000.0f;
static const float LENGTH_SCALE = 1000.0f;

struct Link {
    complex float link_pivot;
    complex float cylinder_pivot;
    float l1, l2;
};

struct GeometryConstants {
    struct Link links[3];
};

static struct GeometryConstants geometry_constants __attribute__ ((section (".storage.linearize"))) = {
    .links = {
        { //CURL
            .cylinder_pivot = 4.287 - 0.44I,
            .l1 = 2.5,
            .l2 = 8.0,
        },
        { //SWING
            .cylinder_pivot = 3.603 + 2.572I,
            .l1 = 3.90,
            .l2 = 0.0,
        },
        { //LIFT
            .link_pivot = 4.287 + 2.06I,
            .l1 = 4.4,
            .l2 = 6.3,
        }
    }
};

static float commanded_position[3];
static float commanded_angle[3];

void Kinematics_CylinderEdgeLengths(const float joint_angle[JOINT_COUNT],
                                   float cylinder_edge_length[JOINT_COUNT])
{
    cylinder_edge_length[JOINT_LIFT] = cabsf(
            geometry_constants.links[JOINT_LIFT].link_pivot -
            geometry_constants.links[JOINT_LIFT].cylinder_pivot -
            geometry_constants.links[JOINT_LIFT].l1 * cexpf(I * joint_angle[JOINT_LIFT]));
    cylinder_edge_length[JOINT_CURL] = cabsf(
            geometry_constants.links[JOINT_LIFT].link_pivot -
            geometry_constants.links[JOINT_CURL].cylinder_pivot +
            geometry_constants.links[JOINT_CURL].l1 * cexpf(I * (joint_angle[JOINT_CURL] + joint_angle[JOINT_LIFT])) +
            geometry_constants.links[JOINT_LIFT].l2 * cexpf(I * joint_angle[JOINT_LIFT]));
    cylinder_edge_length[JOINT_SWING] = cabsf(
            - geometry_constants.links[JOINT_SWING].cylinder_pivot -
            geometry_constants.links[JOINT_SWING].l1 * cexpf(I * joint_angle[JOINT_SWING]));
}

void Kinematics_JointAngles(const float toe[3], float joint_angle[JOINT_COUNT])
{
    complex float T = hypotf(toe[0], toe[1]) + I * toe[2];

    joint_angle[JOINT_CURL] = -acosf(
        (powf(cabsf(T - geometry_constants.links[JOINT_LIFT].link_pivot), 2.0f) -
         (powf(geometry_constants.links[JOINT_LIFT].l2, 2.0f) + 
          powf(geometry_constants.links[JOINT_CURL].l2, 2.0f))) / 
        (2.0f * geometry_constants.links[JOINT_LIFT].l2 * geometry_constants.links[JOINT_CURL].l2));

    joint_angle[JOINT_LIFT] = asinf(
        cimagf((T - geometry_constants.links[JOINT_LIFT].link_pivot) /
               (geometry_constants.links[JOINT_LIFT].l2 +
                geometry_constants.links[JOINT_CURL].l2 * cexpf(I * joint_angle[JOINT_CURL]))));

    joint_angle[JOINT_SWING] = atan2f(toe[1], toe[0]);
}

void Kinematics_ToePosition(const float joint_angle[JOINT_COUNT],
                            float position[3])
{
    complex float T = geometry_constants.links[JOINT_LIFT].link_pivot +
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
    int16_t len;

    if((joint < 0) || (joint > JOINT_COUNT))
        return ILLEGAL_DATA_ADDRESS;
    Linearize_GetJointAngles(joint_angle);
    Kinematics_CylinderEdgeLengths(joint_angle, edge_length);
    len = roundf(LENGTH_SCALE * edge_length[joint]);
    *v = *((uint16_t *)&len);
    return 0;
}

int Kinematics_ReadToePosition(void *ctx, uint16_t *v)
{
    int coordinate = (int)ctx;
    float joint_angle[JOINT_COUNT];
    float toe_postion[3];
    int16_t pos;

    if((coordinate < 0) || (coordinate > 2))
        return ILLEGAL_DATA_ADDRESS;
    Linearize_GetJointAngles(joint_angle);
    Kinematics_ToePosition(joint_angle, toe_postion);
    pos = roundf(LENGTH_SCALE * toe_postion[coordinate]);
    *v = *((uint16_t *)&pos);
    return 0;
}

int Kinematics_WriteToePosition(void *ctx, uint16_t v)
{
    int coordinate = (int)ctx;
    float joint_angle[3];
    float cylinder_edge_length[3];
    float cylinder_scaled_value[3];
    uint16_t cylinder_command[3];

    if((coordinate < 0) || (coordinate > 2))
        return ILLEGAL_DATA_ADDRESS;
    commanded_position[coordinate] = (int16_t)v / LENGTH_SCALE;
    if(coordinate == 2)
    {
        errno = 0;
        Kinematics_JointAngles(commanded_position, joint_angle);
        Kinematics_CylinderEdgeLengths(joint_angle, cylinder_edge_length);
        Linearize_ScaleCylinders(cylinder_edge_length, cylinder_scaled_value);
        if(errno != 0)
        {
            return ILLEGAL_DATA_VALUE;
        }
        for(int j=0;j<JOINT_COUNT;j++)
            cylinder_command[j] = cylinder_scaled_value[j] * 4095;
        Enfield_SetCommand(cylinder_command);
    }
    return 0;
}

int Kinematics_ReadJointAngle(void *ctx, uint16_t *v)
{
    int coordinate = (int)ctx;
    float joint_angle[JOINT_COUNT];
    int16_t pos;

    if((coordinate < 0) || (coordinate > 2))
        return ILLEGAL_DATA_ADDRESS;
    Linearize_GetJointAngles(joint_angle);
    pos = roundf(JOINT_ANGLE_SCALE * joint_angle[coordinate]);
    *v = *((uint16_t *)&pos);
    return 0;
}

int Kinematics_WriteJointAngle(void *ctx, uint16_t v)
{
    int joint = (int)ctx;
    float cylinder_edge_length[3];
    float cylinder_scaled_value[3];
    uint16_t cylinder_command[3];

    if((joint < 0) || (joint > 2))
        return ILLEGAL_DATA_ADDRESS;
    commanded_angle[joint] = (int16_t)v / JOINT_ANGLE_SCALE;
    if(joint == 2)
    {
        errno = 0;
        Kinematics_CylinderEdgeLengths(commanded_angle, cylinder_edge_length);
        Linearize_ScaleCylinders(cylinder_edge_length, cylinder_scaled_value);
        if(errno != 0)
        {
            return ILLEGAL_DATA_VALUE;
        }
        for(int j=0;j<JOINT_COUNT;j++)
            cylinder_command[j] = cylinder_scaled_value[j] * 4095;
        Enfield_SetCommand(cylinder_command);
    }
    return 0;
}
