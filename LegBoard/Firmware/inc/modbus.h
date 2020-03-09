#pragma once

#include <stdint.h>
#include <stdbool.h>

enum ModbusExceptionCode {
// 01 ILLEGAL FUNCTION CODE The function code received in the query is not an
// allowable action for the slave.  If a Poll Program Complete command was
// issued, this code indicates that no program function preceded it.
    ILLEGAL_FUNCTION_CODE = 1,
// 02 ILLEGAL DATA ADDRESS The data address received in the query is not an
// allowable address for the slave.
    ILLEGAL_DATA_ADDRESS  = 2,
// 03 ILLEGAL DATA VALUE A value contained in the query data field is not an
// allowable value for the slave.
    ILLEGAL_DATA_VALUE    = 3,
// 04 SLAVE DEVICE FAILURE An unrecoverable error occurred while the slave was
// attempting to perform the requested action.
    SLAVE_DEVICE_FAILURE  = 4,
// 05 ACKNOWLEDGE The slave has accepted the request and is processing it, but a
// long duration of time will be required to do so. This response is returned to
// prevent a timeout error from occurring in the master. The master can next
// issue a Poll Program Complete message to determine if processing is
// completed.
    ACKNOWLEDGE           = 5,
// 06 SLAVE DEVICE BUSY The slave is engaged in processing a long–duration
// program command. The master should retransmit the message later when the
// slave is free
    SLAVE_DEVICE_BUSY     = 6,
};


struct MODBUS_Coil
{
    uint16_t address;
    void *context;
    int (*read)(void *, bool *);
    int (*write)(void *, bool);
};

struct MODBUS_DiscreteInput
{
    uint16_t address;
    void *context;
    int (*read)(void *, bool *);
};

struct MODBUS_InputRegister
{
    uint16_t address;
    void *context;
    int (*read)(void *, uint16_t *);
};

struct MODBUS_HoldingRegister
{
    uint16_t address;
    void *context;
    int (*read)(void *, uint16_t *);
    int (*write)(void *, uint16_t);
};

void MODBUS_Init(void);

uint16_t MODBUS_crc(uint8_t *data, uint16_t length);
