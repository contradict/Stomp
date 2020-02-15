#include "ads57x4.h"

static uint32_t DAC_REGISTER[4];
static uint32_t RANGE_REGISTER[4];
static uint32_t CONTROL_REGISTER;
static uint32_t CLEAR_REGISTER;
static uint32_t LOAD_REGISTER;
static uint32_t POWER_REGISTER;

int ads57x4_Init(void)
{
    DAC_IO_Init();
    DAC_IO_CLR(false);
    DAC_IO_LDAC(true);
    return 0;
}

int ads5724_SetVoltage(enum ads57x4_channel channel, int16_t volts)
{
    DAC_REGISTER[channel] = (ADS57x4_WRITE |
                             (ADS57x4_REGISTER_DAC << ADS57x4_OFFSET_REGISTER) |
                             (channel << ADS57x4_OFFSET_CHANNEL) |
                             (volts << ADS5724_OFFSET_DATA));
    DAC_IO_Write(&DAC_REGISTER[channel]);
    return 0;
}

int ads57x4_SelectOutputRange(enum ads57x4_channel channel,
                              enum ads57x4_output_range range)
{
    RANGE_REGISTER[channel] = (ADS57x4_WRITE |
                               (ADS57x4_REGISTER_RANGE_SELECT << ADS57x4_OFFSET_REGISTER) |
                               (channel << ADS57x4_OFFSET_CHANNEL) |
                               (range));
    DAC_IO_Write(&RANGE_REGISTER[channel]);
    return 0;
}

int ads57x4_Clear()
{
    CLEAR_REGISTER = (ADS57x4_WRITE |
                      (ADS57x4_REGISTER_CONTROL << ADS57x4_OFFSET_REGISTER) |
                      (ADS57x4_COMMAND_CLEAR << ADS57x4_OFFSET_COMMAND));
    DAC_IO_Write(&CLEAR_REGISTER);
    return 0;
}

int ads57x4_Load()
{
    LOAD_REGISTER = (ADS57x4_WRITE |
                     (ADS57x4_REGISTER_CONTROL << ADS57x4_OFFSET_REGISTER) |
                     (ADS57x4_COMMAND_LOAD << ADS57x4_OFFSET_COMMAND));
    DAC_IO_Write(&LOAD_REGISTER);
    return 0;
}

int ads57x4_Configure(bool TSD, bool Clamp, enum ads57x4_clear_select Clear, bool SDO)
{
    CONTROL_REGISTER = (ADS57x4_WRITE |
                        (ADS57x4_REGISTER_CONTROL << ADS57x4_OFFSET_REGISTER) |
                        (ADS57x4_COMMAND_CONFIGURE << ADS57x4_OFFSET_COMMAND) |
                        (TSD ? (1<<ADS57x4_OFFSET_TSD) : 0) |
                        (Clamp ? (1<<ADS57x4_OFFSET_CLAMP) : 0) |
                        (Clear << ADS57x4_OFFSET_CLR) |
                        (SDO ? (1<<ADS57x4_OFFSET_SDO) : 0));
    DAC_IO_Write(&CONTROL_REGISTER);
    return 0;
}

int ads57x4_Power(uint8_t power)
{
    POWER_REGISTER = (ADS57x4_WRITE |
                      (ADS57x4_REGISTER_POWER_CONTROL << ADS57x4_OFFSET_REGISTER) |
                      (power & 0x0f));
    DAC_IO_Write(&POWER_REGISTER);
    return 0;
}

int ads57x4_Read(enum ads57x4_register reg, enum ads57x4_channel channel, uint32_t* result)
{
    uint32_t readback = (ADS57x4_READ |
                         (reg << ADS57x4_OFFSET_REGISTER) | 
                         (channel << ADS57x4_OFFSET_CHANNEL));
    DAC_IO_Write(&readback);
    DAC_IO_WaitForTransfer();
    readback = (ADS57x4_WRITE | 
                (ADS57x4_REGISTER_CONTROL << ADS57x4_OFFSET_REGISTER) |
                (ADS57x4_COMMAND_NOP << ADS57x4_OFFSET_COMMAND));
    DAC_IO_ReadWrite(&readback, result);
    DAC_IO_WaitForTransfer();
    return 0;
}
