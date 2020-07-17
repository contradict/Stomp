#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <modbus.h>
#include "modbus_device.h"
#include "modbus_register_map.h"

static const char *joint_names[3]={"CURL", "SWING", "LIFT"};
static const int joint_names_length[3]={4, 5, 4};
static const int joint_base[3] = {CURL_BASE, SWING_BASE, LIFT_BASE};


int parse_joint(char *arg)
{
    int count, value;
    count = sscanf(arg, "%d", &value);
    if(count == 1)
    {
        if(value>=0 && value<3)
            return value;
        else
            return -1;
    }
    for(int i=0;i<3;i++)
    {
        if(strncasecmp(arg, joint_names[i], joint_names_length[i]) == 0)
            return i;
    }
    return -1;
}

struct CoilName {
    enum CoilOffset offset;
    const char *name;
};
struct CoilName coilNames[7] = {
    {.offset=CFeedbackPolarity, .name="CFeedbackPolarity"},
    {.offset=CPortConnection, .name="CPortConnection"},
    {.offset=CCommandInput, .name="CCommandInput"},
    {.offset=CFeedbackInput, .name="CFeedbackInput"},
    {.offset=CCommandSource, .name="CCommandSource"},
    {.offset=CZeroGain, .name="CZeroGain"},
    {.offset=CSaveConfiguration, .name="CSaveConfiguration"},
};
int find_coil(char *arg)
{
    for(int i=0;i<7;i++)
    {
        if(strcmp(arg, coilNames[i].name) == 0)
            return coilNames[i].offset;
    }
    return -1;
}

struct InputRegisterName {
    enum InputRegisterOffset offset;
    const char *name;
};
struct InputRegisterName inputRegisterNames[18] = {
    {.offset=ISensorVoltage, .name="ISensorVoltage"},
    {.offset=IJointAngle, .name="IJointAngle"},
    {.offset=IFeedbackVoltage, .name="IFeedbackVoltage"},
    {.offset=ICylinderLength, .name="ICylinderLength"},
    {.offset=ICachedBaseEndPressure, .name="ICachedBaseEndPressure"},
    {.offset=ICachedRodEndPressure, .name="ICachedRodEndPressure"},
    {.offset=ICachedFeedbackPosition, .name="ICachedFeedbackPosition"},
    {.offset=ISerialNumberLo, .name="ISerialNumberLo"},
    {.offset=ISerialNumberHi, .name="ISerialNumberHi"},
    {.offset=IAnalogCommand, .name="IAnalogCommand"},
    {.offset=IFeedbackPosition, .name="IFeedbackPosition"},
    {.offset=IBaseEndPressure, .name="IBaseEndPressure"},
    {.offset=IRodEndPressure, .name="IRodEndPressure"},
    {.offset=ISpoolPosition, .name="ISpoolPosition"},
    {.offset=IFirmwareVersionB, .name="IFirmwareVersionB"},
    {.offset=IFirmwareVersionA, .name="IFirmwareVersionA"},
    {.offset=IFirmwareRevLow, .name="IFirmwareRevLow"},
    {.offset=IFirmwareRevHigh, .name="IFirmwareRevHigh"},
};
int find_input_register(char *arg)
{
    for(int i=0;i<18;i++)
    {
        if(strcmp(arg, inputRegisterNames[i].name) == 0)
            return inputRegisterNames[i].offset;
    }
    return -1;
}


struct HoldingRegisterName {
    enum HoldingRegisterOffset offset;
    const char *name;
};
struct HoldingRegisterName holdingRegisterNames[21] = {
    {.offset=HSensorVmin, .name="HSensorVmin"},
    {.offset=HSensorVmax, .name="HSensorVmax"},
    {.offset=HSensorThetamin, .name="HSensorThetamin"},
    {.offset=HSensorThetamax, .name="HSensorThetamax"},
    {.offset=HCylinderLengthMin, .name="HCylinderLengthMin"},
    {.offset=HCylinderLengthMax, .name="HCylinderLengthMax"},
    {.offset=HProportionalGain, .name="HProportionalGain"},
    {.offset=HDerivativeGain, .name="HDerivativeGain"},
    {.offset=HForceDamping, .name="HForceDamping"},
    {.offset=HOffset, .name="HOffset"},
    {.offset=HAreaRatio, .name="HAreaRatio"},
    {.offset=HCylinderBore, .name="HCylinderBore"},
    {.offset=HMinimumPosition, .name="HMinimumPosition"},
    {.offset=HMaximumPosition, .name="HMaximumPosition"},
    {.offset=HRampUp, .name="HRampUp"},
    {.offset=HRampDown, .name="HRampDown"},
    {.offset=HDeadBand, .name="HDeadBand"},
    {.offset=HDitherAmplitude, .name="HDitherAmplitude"},
    {.offset=HValveOffset, .name="HValveOffset"},
    {.offset=HDigitalCommand, .name="HDigitalCommand"},
    {.offset=HCachedDigitalCommand, .name="HCachedDigitalCommand"},
};
int find_holding_register(char *arg)
{
    for(int i=0;i<21;i++)
    {
        if(strcmp(arg, holdingRegisterNames[i].name) == 0)
            return holdingRegisterNames[i].offset;
    }
    return -1;
}


struct LegCommandName {
    enum LegCommands offset;
    const char *name;
};
struct LegCommandName legCommandNames[6] = {
    {.offset=ToeXPosition, .name="ToeXPosition"},
    {.offset=ToeYPosition, .name="ToeYPosition"},
    {.offset=ToeZPosition, .name="ToeZPosition"},
    {.offset=JointAngleCurl, .name="JointAngleCurl"},
    {.offset=JointAngleSwing, .name="JointAngleSwing"},
    {.offset=JointAngleLift, .name="JointAngleLift"},
};
int find_leg(char *arg)
{
    for(int i=0;i<6;i++)
    {
        if(strcmp(arg, legCommandNames[i].name) == 0)
            return legCommandNames[i].offset;
    }
    return -1;
}


int parse_register(char *arg)
{
    int count=0, value;
    char *offset;

    offset = strchr(arg, 'x');
    if(offset)
    {
        count = sscanf(offset + 1, "%x", &value);
    }
    else
    {
        count = sscanf(arg+1, "%d", &value);
    }
    if(count == 1)
    {
        return value;
    }
    char type = arg[0];
    switch(type)
    {
        case 'I':
            return find_input_register(arg);
            break;
        case 'H':
            return find_holding_register(arg);
            break;
        case 'C':
            return find_coil(arg);
            break;
        case 'T':
        case 'J':
            return find_leg(arg);
            break;
        default:
            return -1;
            break;
    }
}

void read_serialnumber(modbus_t *ctx, int joint)
{
    uint16_t serial_number[2];

    uint16_t base = joint_base[joint];
    printf("base: 0x%x\n", base);

    bzero(serial_number, sizeof(serial_number));
    if(modbus_read_input_registers(ctx, base + ISerialNumberLo, 2, serial_number) == -1)
        printf("  Read serial number failed: %s\n", modbus_strerror(errno));
    else
        printf("  Serial Number: %d\n", *(uint32_t *)serial_number);
}

int checkjoint(int joint)
{
    if(joint<0)
    {
        printf("Must specify joint (-j)\n");
        exit(1);
    }
    return joint_base[joint];
}

int main(int argc, char **argv)
{
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    int addr = 0x55;
    uint32_t response_timeout = 2000;

    int opt;
    char *offset;
    bool read_serial = false;
    int joint = -1;
    int16_t read_register=-1, write_register=-1;
    uint16_t write_value;
    char read_type, write_type;
    char *wreg_ptr, *saveptr;

    while((opt = getopt(argc, argv, "p:b:a:r:j:sR:W:")) != -1)
    {
        switch(opt)
        {
            case 'p':
                devname = strdup(optarg);
                break;
            case 'b':
                baud = atol(optarg);
                break;
            case 'a':
                offset = strchr(optarg, 'x');
                if(offset)
                {
                    sscanf(offset + 1, "%x", &addr);
                }
                else
                {
                    addr = atoi(optarg);
                }
                printf("address: 0x%x\n", addr);
                break;
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
            case 'j':
                joint = parse_joint(optarg);
                break;
            case 's':
                read_serial = true;
                break;
            case 'R':
                read_register = parse_register(optarg);
                read_type = optarg[0];
                break;
            case 'W':
                wreg_ptr = strtok_r(optarg, ":", &saveptr);
                write_register = parse_register(wreg_ptr);
                write_type = optarg[0];
                wreg_ptr = strtok_r(NULL, ":", &saveptr);
                offset = strchr(wreg_ptr, 'x');
                if(offset)
                {
                    if(sscanf(offset + 1, "%hx", &write_value) != 1)
                    {
                        printf("Unable to parse hex value %s\n", offset);
                        exit(1);
                    }
                }
                else
                {
                    write_value = atoi(wreg_ptr);
                }
                break;
        }
    }

    uint8_t address = 0xFF & addr;

    modbus_t *ctx;

    if(create_modbus_interface(devname, baud, response_timeout, &ctx))
    {
        exit(1);
    }

    modbus_set_slave(ctx, address);

    if(read_serial)
    {
        if(joint > 0)
            read_serialnumber(ctx, joint);
        else
            for(int j=0; j<3; j++)
            {
                read_serialnumber(ctx, j);
                usleep(500000);
            }
    }

    uint16_t base;
    int err;

    if(write_register >= 0)
    {
        uint8_t write_bit;
        switch(write_type)
        {
            case 'C':
                base = checkjoint(joint);
                write_bit = write_value;
                err = modbus_write_bits(ctx, base + write_register, 1, &write_bit);
                if(err == -1)
                {
                    printf("Unable to write joint %d, register 0x%x: %s\n", joint, write_register, modbus_strerror(errno));
                }
                break;
            case 'H':
                base = checkjoint(joint);
                err = modbus_write_registers(ctx, base + write_register, 1, &write_value);
                if(err == -1)
                {
                    printf("Unable to write joint %d, register 0x%x: %s\n", joint, write_register, modbus_strerror(errno));
                }
                else
                {
                    printf("Wrote joint %d, register 0x%x = 0x%x\n", joint, write_register, write_value);
                }
                break;
            case 'T':
            case 'J':
                printf("Writing register 0x%x = 0x%x\n", write_register, write_value);
                err = modbus_write_registers(ctx, write_register, 1, &write_value);
                if(err == -1)
                {
                    printf("Unable to write register 0x%x = 0x%x: %s\n", write_register, write_value, modbus_strerror(errno));
                }
                else
                {
                    printf("Wrote register 0x%x = 0x%x\n", write_register, write_value);
                }
                break;
         }
    }

    if(read_register >= 0)
    {
        uint16_t value;
        uint8_t bit_value;
        switch(read_type)
        {
            case 'C':
                base = checkjoint(joint);
                err = modbus_read_bits(ctx, base + read_register, 1, &bit_value);
                if(err == -1)
                {
                    printf("Unable to read joint %d, register 0x%x: %s\n", joint, read_register, modbus_strerror(errno));
                }
                else
                {
                    printf("Joint %d, register 0x%x = 0x%x\n",
                            joint, read_register, bit_value);
                }
                break;
            case 'H':
                base = checkjoint(joint);
                err = modbus_read_registers(ctx, base + read_register, 1, &value);
                if(err == -1)
                {
                    printf("Unable to read joint %d, register 0x%x: %s\n", joint, read_register, modbus_strerror(errno));
                }
                else
                {
                    printf("Joint %d, register 0x%x = 0x%x\n",
                            joint, read_register, value);
                }
                break;
            case 'J':
            case 'T':
                err = modbus_read_registers(ctx, read_register, 1, &value);
                if(err == -1)
                {
                    printf("Unable to read joint %d, register 0x%x: %s\n", joint, read_register, modbus_strerror(errno));
                }
                else
                {
                    printf("Joint %d, register 0x%x = 0x%x\n",
                            joint, read_register, value);
                }
                break;
         }
    }

    modbus_close(ctx);
    return 0;
}
