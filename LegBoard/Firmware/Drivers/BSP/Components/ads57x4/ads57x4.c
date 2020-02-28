#include "ads57x4.h"

#define ADS57x4_WRITE 0
#define ADS57x4_READ (1<<7)

#define ADS5754_OFFSET_DATA 0
#define ADS5734_OFFSET_DATA 2
#define ADS5724_OFFSET_DATA 4

#define ADS57x4_OFFSET_TSD 3
#define ADS57x4_OFFSET_CLAMP 2
#define ADS57x4_OFFSET_CLR 1
#define ADS57x4_OFFSET_SDO 0

#define ADS57x4_OFFSET_REGISTER 3
#define ADS57x4_OFFSET_CHANNEL 0

#define HIGH_BYTE(x) ((x&0xFF00)>>8)
#define LOW_BYTE(x) (x&0x00FF)

#define WREGISTER(r, c) (ADS57x4_WRITE | (r << ADS57x4_OFFSET_REGISTER) | (c << ADS57x4_OFFSET_CHANNEL))
#define RREGISTER(r, c) (ADS57x4_READ | (r << ADS57x4_OFFSET_REGISTER) | (c << ADS57x4_OFFSET_CHANNEL))


extern int DAC_IO_Init();
extern void DAC_IO_Write(uint8_t write[3]);
extern void DAC_IO_ReadWrite(uint8_t write[3], uint8_t read[3]);
extern void DAC_IO_WaitForTransfer(void);
extern void DAC_IO_LDAC(bool state);
extern void DAC_IO_CLR(bool state);
extern void DAC_IO_TransferComplete();


static uint8_t DAC_REGISTER[4][3];
static uint8_t RANGE_REGISTER[4][3];
static uint8_t CONTROL_REGISTER[3];
static uint8_t CLEAR_REGISTER[3];
static uint8_t LOAD_REGISTER[3];
static uint8_t POWER_REGISTER[3];


int ads57x4_Init(void)
{
    DAC_IO_Init();
    DAC_IO_CLR(false);
    DAC_IO_LDAC(true);
    return 0;
}

int ads5724_SetVoltage(enum ads57x4_channel channel, int16_t volts)
{
    DAC_REGISTER[channel][0] = WREGISTER(ADS57x4_REGISTER_DAC, channel);
    DAC_REGISTER[channel][1] = HIGH_BYTE(volts);
    DAC_REGISTER[channel][2] = LOW_BYTE(volts);
    DAC_IO_Write(DAC_REGISTER[channel]);
    return 0;
}

int ads57x4_SelectOutputRange(enum ads57x4_channel channel,
                              enum ads57x4_output_range range)
{
    RANGE_REGISTER[channel][0] = WREGISTER(ADS57x4_REGISTER_RANGE_SELECT, channel);
    RANGE_REGISTER[channel][1] = 0;
    RANGE_REGISTER[channel][2] = range;
    DAC_IO_Write(RANGE_REGISTER[channel]);
    return 0;
}

int ads57x4_Clear()
{
    CLEAR_REGISTER[0] = WREGISTER(ADS57x4_REGISTER_CONTROL, ADS57x4_COMMAND_CLEAR);
    CLEAR_REGISTER[1] = 0;
    CLEAR_REGISTER[2] = 0;
    DAC_IO_Write(CLEAR_REGISTER);
    return 0;
}

int ads57x4_Load()
{
    LOAD_REGISTER[0] = WREGISTER(ADS57x4_REGISTER_CONTROL, ADS57x4_COMMAND_LOAD);
    LOAD_REGISTER[1] = 0;
    LOAD_REGISTER[2] = 0;
    DAC_IO_Write(LOAD_REGISTER);
    return 0;
}

int ads57x4_Configure(bool TSD, bool Clamp, enum ads57x4_clear_select Clear, bool SDO)
{
    CONTROL_REGISTER[0] = WREGISTER(ADS57x4_REGISTER_CONTROL, ADS57x4_COMMAND_CONFIGURE);
    CONTROL_REGISTER[1] = 0;
    CONTROL_REGISTER[2] = ((TSD ? (1<<ADS57x4_OFFSET_TSD) : 0) |
                           (Clamp ? (1<<ADS57x4_OFFSET_CLAMP) : 0) |
                           (Clear << ADS57x4_OFFSET_CLR) |
                           (SDO ? (1<<ADS57x4_OFFSET_SDO) : 0));
    DAC_IO_Write(CONTROL_REGISTER);
    return 0;
}

int ads57x4_Power(uint8_t power)
{
    POWER_REGISTER[0] = WREGISTER(ADS57x4_REGISTER_POWER_CONTROL, 0);
    POWER_REGISTER[1] = 0;
    POWER_REGISTER[2] = (power & 0x0f);
    DAC_IO_Write(POWER_REGISTER);
    return 0;
}

int ads57x4_Read(enum ads57x4_register reg, enum ads57x4_channel channel, uint8_t result[3])
{
    uint8_t readback[3] = {RREGISTER(reg, channel), 0, 0};
    DAC_IO_Write(readback);
    DAC_IO_WaitForTransfer();
    readback[0] = WREGISTER(ADS57x4_REGISTER_CONTROL, ADS57x4_COMMAND_NOP);
    readback[1] = 0;
    readback[2] = 0;
    DAC_IO_ReadWrite(readback, result);
    DAC_IO_WaitForTransfer();
    return 0;
}
