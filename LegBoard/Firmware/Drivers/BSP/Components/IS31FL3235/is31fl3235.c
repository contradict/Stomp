#include <string.h>
#include "is31fl3235.h"

#define IS31FL3235_NUM_CHANNELS 28

#define IS31FL3235_ADDR_GND 0x78
#define IS31FL3235_ADDR_VCC 0x7E
#define IS31FL3235_ADDR_SCL 0x7A
#define IS31FL3235_ADDR_SDA 0x7C

#define IS31FL3235_REG_SHUTDOWN      0x00
#define IS31FL3235_REG_PWM(x)        (0x05 + x)
#define IS31FL3235_REG_UPDATE        0x25
#define IS31FL3235_REG_LEDCONTROL(x) (0x2A + x)
#define IS31FL3235_REG_CONTROL       0x4A
#define IS31FL3235_REG_RESET         0x4F


#define IS31FL3235_SHUTDOWN_SWSHDN   0x00
#define IS31FL3235_SHUTDOWN_NORM     0x01

#define IS31FL3235_UPDATE_GO 0x00

#define IS31FL3235_LED_CONTROL_CURRENT_IMAX  0x00
#define IS31FL3235_LED_CONTROL_CURRENT_IMAX2 0x02
#define IS31FL3235_LED_CONTROL_CURRENT_IMAX3 0x04
#define IS31FL3235_LED_CONTROL_CURRENT_IMAX4 0x06
#define IS31FL3235_LED_CONTROL_OUT_OFF       0x00
#define IS31FL3235_LED_CONTROL_OUT_ON        0x01

#define IS31FL3235_CONTROL_NORMAL 0x00
#define IS31FL3235_CONTROL_SHDN   0x00

#define IS31FL3235_RESET_RESET    0x00

extern void LED_I2C_Init(void);
extern void LED_I2C_Write(uint16_t address, uint8_t *data, uint16_t size);

static uint16_t i2c_address;
static bool request_go = false;
static uint8_t SHUTDOWN[2] = {IS31FL3235_REG_SHUTDOWN, IS31FL3235_SHUTDOWN_NORM};
static uint8_t UPDATE[2]   = {IS31FL3235_REG_UPDATE,   IS31FL3235_UPDATE_GO};
static uint8_t CONTROL[2]  = {IS31FL3235_REG_CONTROL,  IS31FL3235_CONTROL_NORMAL};
static uint8_t RESET[2]    = {IS31FL3235_REG_RESET,    IS31FL3235_RESET_RESET};

static uint8_t led_pwm[1+IS31FL3235_NUM_CHANNELS];
static uint8_t led_control[1+IS31FL3235_NUM_CHANNELS];

void is31fl3235_Init(uint16_t address)
{
    LED_I2C_Init();
    i2c_address = address;
}

void is31fl3235_Reset()
{
    LED_I2C_Write(i2c_address, (uint8_t*)&RESET, 2);
}

void is31fl3235_Shutdown(bool shutdown)
{
    SHUTDOWN[1] = (shutdown ? IS31FL3235_SHUTDOWN_SWSHDN : IS31FL3235_SHUTDOWN_NORM);
    LED_I2C_Write(i2c_address, (uint8_t*)&SHUTDOWN, 2);
}

void is31fl3235_Update()
{
    LED_I2C_Write(i2c_address, (uint8_t*)&UPDATE, 2);
}

void is31fl3235_Enable(bool enable)
{
    CONTROL[1] = enable ? IS31FL3235_CONTROL_NORMAL : IS31FL3235_CONTROL_SHDN;
    LED_I2C_Write(i2c_address, (uint8_t*)&CONTROL, 2);
}

void is31fl3235_Set(uint8_t start_channel, uint8_t num_channels, uint8_t *data)
{
    led_pwm[0] = IS31FL3235_REG_PWM(start_channel);
    memcpy(led_pwm + 1, data, num_channels);
    request_go = true;
    LED_I2C_Write(i2c_address, led_pwm, num_channels + 1);
}

void is31fl3235_SetControl(uint8_t start_channel, uint8_t num_channels, uint8_t *data)
{
    led_control[0] = IS31FL3235_REG_PWM(start_channel);
    memcpy(led_control + 1, data, num_channels);
    LED_I2C_Write(i2c_address, led_control, num_channels + 1);
}

void LED_I2C_Complete(void)
{
    if(request_go)
    {
        is31fl3235_Update();
    }
    request_go = false;
}
