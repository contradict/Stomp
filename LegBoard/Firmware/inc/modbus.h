#pragma once

#include <stdint.h>
#include <stdbool.h>

struct MODBUS_Coil
{
    uint16_t address;
    void *context;
    bool (*read)(void *);
    void (*write)(void *, bool);
};

struct MODBUS_DiscreteInput
{
    uint16_t address;
    void *context;
    bool (*read)(void *);
};

struct MODBUS_InputRegister
{
    uint16_t address;
    void *context;
    uint16_t (*read)(void *);
};

struct MODBUS_HoldingRegister
{
    uint16_t address;
    void *context;
    uint16_t (*read)(void *);
    void (*write)(void *, uint16_t);
};


void MODBUS_Init(void);
