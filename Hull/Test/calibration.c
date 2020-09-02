#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <modbus.h>

#include <toml.h>

#include "modbus_device.h"
#include "modbus_register_map.h"

bool parse_number(char *arg, uint16_t *value)
{
    int count=0;
    char *offset;

    offset = strchr(arg, 'x');
    if(offset)
    {
        count = sscanf(offset + 1, "%hx", value);
    }
    else
    {
        count = sscanf(arg, "%hd", value);
    }
    if(count == 1)
    {
        return true;
    }
    return false;
}

void ensure_ctx(modbus_t **ctx, char *devname, uint32_t baud, uint32_t byte_timeout, uint32_t response_timeout, uint8_t address)
{
    if(*ctx == NULL)
    {
        if(create_modbus_interface(devname, baud, byte_timeout, response_timeout, ctx))
        {
            exit(1);
        }
        modbus_set_slave(*ctx, address);
        //modbus_set_debug(*ctx, 1);
    }
}

void save_values(modbus_t* ctx, int startleg, int endleg, toml_table_t* config, bool save)
{
    toml_table_t* registers = toml_table_in(config, "calibration_registers");
    if(registers == 0)
    {
        printf("No calibration_registers table.\n");
        return;
    }
    toml_table_t* joints = toml_table_in(config, "joints");
    if(joints == 0)
    {
        printf("No joint offsets table.\n");
        return;
    }
    toml_array_t* leg = toml_array_in(config, "leg");
    for(int i=0; i<toml_array_nelem(leg); i++)
    {
        toml_table_t* currleg = toml_table_at(leg, i);
        toml_raw_t r = toml_raw_in(currleg, "index");
        int64_t idx;
        int err = toml_rtoi(r, &idx);
        if(err < 0)
        {
            printf("Could not parse index for leg %d: %s.", i, (char *)r);
            continue;
        }
        if(idx>=startleg && idx<=endleg)
        {
            r = toml_raw_in(currleg, "address");
            if(r == 0)
            {
                printf("No address for leg %d, idx=%ld.\n", i, idx);
                continue;
            }
            int64_t addr;
            err = toml_rtoi(r, &addr);
            if(err < 0)
            {
                printf("Could not parse address for leg %d idx=%ld: %s.\n",
                       i, idx, (char*)r);
                continue;
            }
            uint8_t address = addr & 0xFF;
            modbus_set_slave(ctx, address);
            toml_array_t* joint=toml_array_in(currleg, "joint");
            if(joint == 0)
            {
                printf("No joint table in leg %d idx=%ld.\n", i, idx);
                continue;
            }
            for(int j=0; j<toml_array_nelem(joint); j++)
            {
                toml_table_t* currjoint = toml_table_at(joint, j);
                r = toml_raw_in(currjoint, "name");
                if(r == 0)
                {
                    printf("No name for joint %d leg %d idx=%ld.\n", j, i, idx);
                    continue;
                }
                char *jname;
                err = toml_rtos(r, &jname);
                if(err < 0)
                {
                    printf("Unable to parse joint name %d leg %d idx=%ld: %s.\n",
                           j, i, idx, (char *)r);
                    continue;
                }
                toml_table_t* settings = toml_table_in(currjoint, "settings");
                if(settings == 0)
                {
                    printf("No settings for joint %s(%d) leg %d idx=%ld.\n",
                            jname, j, i, idx);
                    free(jname);
                    continue;
                }
                toml_table_t* jdesc = toml_table_in(joints, jname);
                if(jdesc == 0)
                {
                    printf("No joint description for joint name %s. "
                           "Found in joint %d leg %d idx=%ld", jname, j, i, idx);
                    free(jname);
                    continue;
                }
                r = toml_raw_in(jdesc, "offset");
                if(r == 0)
                {
                    printf("No offset defined for joint %s.\n", jname);
                    free(jname);
                    continue;
                }
                int64_t joffset;
                err = toml_rtoi(r, &joffset);
                if(err < 0)
                {
                    printf("Unable to parse joint offset for joint %s: %s.\n",
                           jname, (char*)r);
                    free(jname);
                    continue;
                }
                for(int s=0; s<toml_table_nkval(settings); s++)
                {
                    const char* regname = toml_key_in(settings, s);
                    toml_table_t* regdesc = toml_table_in(registers, regname);
                    if(regdesc == 0)
                    {
                        printf("No calibration_register definition for register named %s. "
                               "Found in joint %d leg %d idx=%ld", regname, j, i, idx);
                        continue;
                    }
                    r = toml_raw_in(regdesc, "address");
                    if(r == 0)
                    {
                        printf("Register %s has no address.\n", regname);
                        continue;
                    }
                    int64_t regaddr;
                    err = toml_rtoi(r, &regaddr);
                    if(err < 0)
                    {
                        printf("Unable to parse register %s address: %s.\n",
                               regname, (char*)r);
                        continue;
                    }
                    r = toml_raw_in(regdesc, "scale");
                    if(r == 0)
                    {
                        printf("Register %s has no scale.\n", regname);
                        continue;
                    }
                    double regscale;
                    err = toml_rtod(r, &regscale);
                    if(err < 0)
                    {
                        printf("Unable to parse scale for register %s: %s.\n",
                                regname, (char*)r);
                        continue;
                    }
                    int16_t fulladdr = regaddr + joffset;
                    if(save)
                    {
                        uint16_t currentval;
                        err = modbus_read_registers(ctx, fulladdr, 1, &currentval);
                        if(err < 0)
                        {
                            printf("Failed to read register %s(0x%02x) joint %s(%d) leg %d idx=%ld: %s\n",
                                    regname, fulladdr, jname, j, i, idx, modbus_strerror(errno));
                            continue;
                        }
                        r = toml_dtor(currentval / regscale);
                        err = toml_raw_set(settings, regname, r);
                        if(err < 0)
                        {
                            printf("Failed to save register %s(0x%02x) joint %s(%d) leg %d idx=%ld.\n",
                                    regname, fulladdr, jname, j, i, idx);
                        }
                    }
                    else
                    {
                        r = toml_raw_in(settings, regname);
                        double regval;
                        err = toml_rtod(r, &regval);
                        if(err < 0)
                        {
                            printf("Unable to parse value for register %s, joint %s(%d), leg %d idx=%ld: %s.\n",
                                    regname, jname, j, i, idx, (char*)r);
                            continue;
                        }
                        int16_t scaledval = regval * regscale;
                        err = modbus_write_registers(ctx, fulladdr, 1, (uint16_t*)&scaledval);
                        if(err < 0)
                        {
                            printf("Failed to write register %s(0x%02x) joint %s(%d) leg %d idx=%ld: %s\n",
                                    regname, fulladdr, jname, j, i, idx, modbus_strerror(errno));
                            continue;
                        }
                    }
                }
                free(jname);
            }
        }
    }
}

int main(int argc, char **argv)
{
    char *devname = "/dev/ttyS4";
    uint32_t baud = 1000000;
    int addr = 0x55;
    uint32_t response_timeout = 10000;
    uint32_t byte_timeout = 50;
    char *configfile="calibration.toml";

    int opt;
    char *offset;
    int leg=-1, startleg, endleg;

    modbus_t *ctx = NULL;

    while((opt = getopt(argc, argv, "p:b:a:r:t:l:c:")) != -1)
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
                if(ctx != NULL)
                    modbus_set_slave(ctx, 0xff & addr);
                break;
            case 'r':
                response_timeout = 1000*atoi(optarg);
                break;
            case 't':
                byte_timeout = atoi(optarg);
                break;
            case 'l':
                leg = atoi(optarg);
                break;
            case 'c':
                configfile = optarg;
        }
    }

    if(leg>0 && leg<6)
    {
        startleg=leg;
        endleg=leg + 1;
    }
    else
    {
        startleg=0;
        endleg=6;
    }

    FILE* fp;
    char errbuf[200];
    if(0 == (fp = fopen(configfile, "r")))
    {
        snprintf(errbuf, sizeof(errbuf),"Unable to open config file %s:", configfile);
        perror(errbuf);
        exit(1);
    }

    toml_table_t *full_config = toml_parse_file(fp, errbuf, sizeof(errbuf));
    if(0 == full_config)
    {
        printf("Unable to parse %s: %s\n", configfile, errbuf);
        exit(1);
    }
    fclose(fp);

    uint8_t address = addr;
    ensure_ctx(&ctx, devname, baud, byte_timeout, response_timeout, address);

    bool restore = (argv[optind] && strcasecmp(argv[optind], "restore") == 0);
    save_values(ctx, startleg, endleg, full_config, !restore);
    modbus_close(ctx);
    if(!restore)
    {
        if(0 == (fp = fopen(configfile, "w")))
        {
            printf("Unable to open %s for writing: %s\n", configfile, strerror(errno));
        }
        else
        {
            toml_save_file(full_config, fp);
            fclose(fp);
        }
    }

    free(full_config);

    return 0;
}
