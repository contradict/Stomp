#pragma once

#include <stdint.h>
#include <stdbool.h>

#define ADS57x4_WRITE 0
#define ADS57x4_READ (1<<23)

#define ADS57x4_OFFSET_REGISTER 19
#define ADS57x4_OFFSET_CHANNEL 16
#define ADS57x4_OFFSET_COMMAND 16

#define ADS5754_OFFSET_DATA 0
#define ADS5734_OFFSET_DATA 2
#define ADS5724_OFFSET_DATA 4

#define ADS57x4_OFFSET_TSD 3
#define ADS57x4_OFFSET_CLAMP 2
#define ADS57x4_OFFSET_CLR 1
#define ADS57x4_OFFSET_SDO 0

enum ads57x4_channel {
    ADS57x4_CHANNEL_A = 0,
    ADS57x4_CHANNEL_B = 1,
    ADS57x4_CHANNEL_C = 2,
    ADS57x4_CHANNEL_D = 3
};

enum ads57x4_output_range {
    ADS57x4_RANGE_U5V=0,
    ADS57x4_RANGE_U10V=1,
    ADS57x4_RANGE_U10p8V=2,
    ADS57x4_RANGE_B5V=3,
    ADS57x4_RANGE_B10V=4,
    ADS57x4_RANGE_B10p8V=5
};

enum ads57x4_clear_select {
    ADS57x4_CLEAR_ZERO=0,  // Clear to 0V
    ADS57x4_CLEAR_UMID=1,  // Unipolar mode only, clear to midrange
    ADS57x4_CLEAR_BNEG=1   // Bipolar mode only, clear to full negative
};

enum ads57x4_register {
    ADS57x4_REGISTER_DAC=0,
    ADS57x4_REGISTER_RANGE_SELECT=1,
    ADS57x4_REGISTER_POWER_CONTROL=2,
    ADS57x4_REGISTER_CONTROL=3
};

enum ads57x4_command {
    ADS57x4_COMMAND_NOP=0,
    ADS57x4_COMMAND_CONFIGURE=1,
    ADS57x4_COMMAND_CLEAR=4,
    ADS57x4_COMMAND_LOAD=6
};

enum ads57x4_power_config {
    ADS57x4_POWER_PUa=1<<0,
    ADS57x4_POWER_PUb=1<<1,
    ADS57x4_POWER_PUc=1<<2,
    ADS57x4_POWER_PUd=1<<3,
    ADS57x4_POWER_TSD=1<<5,
    ADS57x4_POWER_OCa=1<<7,
    ADS57x4_POWER_OCb=1<<8,
    ADS57x4_POWER_OCc=1<<9,
    ADS57x4_POWER_OCd=1<<10
};

extern int DAC_IO_Init();
extern void DAC_IO_Write(uint32_t* write);
extern void DAC_IO_ReadWrite(uint32_t* write, uint32_t* read);
extern void DAC_IO_WaitForTransfer(void);
extern void DAC_IO_LDAC(bool state);
extern void DAC_IO_CLR(bool state);
extern void DAC_IO_TransferComplete();

int ads57x4_Init();
int ads5724_SetVoltage(enum ads57x4_channel channel, int16_t volts);
int ads57x4_SelectOutputRange(enum ads57x4_channel channel,
                              enum ads57x4_output_range range);
int ads57x4_Clear();
int ads57x4_Load();
int ads57x4_Configure(bool TSD, bool Clamp, enum ads57x4_clear_select Clear, bool SDO);
int ads57x4_Power(uint8_t power);
int ads57x4_Read(enum ads57x4_register, enum ads57x4_channel channel, uint32_t* result);
