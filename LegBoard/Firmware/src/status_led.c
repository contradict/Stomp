#include "cmsis_os.h"
#include "is31fl3235.h"

#define COMMAND_IDX(value) ((value>>24) & 0xFF)
#define COMMAND_R(value) ((value>>16) & 0xFF)
#define COMMAND_G(value) ((value>>8) & 0xFF)
#define COMMAND_B(value) (value & 0xFF)
#define COMMAND_V(value) (value & 0xFF)

#define COMMAND_CODE_RGB 0x00
#define COMMAND_CODE_SINGLE 0x80
#define COMMAND_CHANNEL_R 0x00
#define COMMAND_CHANNEL_G 0x20
#define COMMAND_CHANNEL_B 0x40
#define COMMAND_CHANNEL_RAW 0x60
#define COMMAND_IDX_MASK 0x1f
#define COMMAND_CHANNEL_MASK 0x60
#define COMMAND_CHANNEL_SHIFT 5

#define COMMAND_RGB(idx, r, g, b) (((idx&0x1f) << 24) | ((r&0xff)<<16) | ((g&0xff)<<8) | (b&0xff))
#define COMMAND_S(idx, c, v) ((((idx&0x1f) | c | COMMAND_CODE_SINGLE) << 24) | (v&0xff))
#define COMMAND_RAW(idx, v) ((((idx&0x1f) | COMMAND_CHANNEL_RAW | COMMAND_CODE_SINGLE) << 24) | (v&0xff))

static void LED_Thread(const void *args);
static void RGB_Thread(const void *args);

osThreadDef(led, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
osThreadDef(rgb, RGB_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);

osMessageQDef(ledcommand, 32, uint32_t);

static osThreadId status_led;
static osThreadId rgb_tid;

static osMessageQId commandQ;

void LED_ThreadInit(void)
{
    is31fl3235_Init(IS31FL3235_ADDR_GND);

    status_led = osThreadCreate(osThread(led), NULL);
    commandQ = osMessageCreate(osMessageQ(ledcommand), status_led);
}

static void LED_Thread(const void *args)
{
    (void)args;
    osEvent event;
    uint8_t idx, rgb[3], nrgb;
    uint8_t control_default[9] = {
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON
    };
    while(1)
    {
        is31fl3235_Reset();
        event = osSignalWait(0, 100);
        if(event.status == osEventTimeout)
        {
            continue;
        }
        is31fl3235_Shutdown(false);
        event = osSignalWait(0, 100);
        if(event.status == osEventTimeout)
        {
            continue;
        }
        is31fl3235_SetControl(0, 9, control_default);
        event = osSignalWait(0, 100);
        if(event.status == osEventTimeout)
        {
            continue;
        }
        is31fl3235_Update();
        event = osSignalWait(0, 100);
        if(event.status == osEventTimeout)
        {
            continue;
        }
        break;
    }

    for(;;)
    {
        event = osMessageGet(commandQ, osWaitForever);
        if(event.status != osEventMessage)
        {
            continue;
        }
        idx = COMMAND_IDX(event.value.v);
        if(idx & COMMAND_CODE_SINGLE)
        {
            if((idx & COMMAND_CHANNEL_RAW) == COMMAND_CHANNEL_RAW)
            {
                idx = idx & COMMAND_IDX_MASK;
                rgb[0] = COMMAND_V(event.value.v);
                nrgb = 1;
            }
            else
            {
                idx = (idx & COMMAND_IDX_MASK) + ((idx & COMMAND_CHANNEL_MASK)>>COMMAND_CHANNEL_SHIFT);
                rgb[0] = COMMAND_V(event.value.v);
                nrgb = 1;
            }
        }
        else
        {
            rgb[0] = COMMAND_R(event.value.v);
            rgb[1] = COMMAND_G(event.value.v);
            rgb[2] = COMMAND_B(event.value.v);
            nrgb = 3;
        }
        while(1)
        {
            is31fl3235_Set(idx, nrgb, rgb);
            event = osSignalWait(0, 100);
            if(event.status == osEventTimeout)
            {
                continue;
            }
            is31fl3235_Update();
            event = osSignalWait(0, 100);
            if(event.status == osEventTimeout)
            {
                continue;
            }
            break;
        }
    }
}

void LED(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    osMessagePut(commandQ, COMMAND_RGB(idx * 3, r, g, b), 0);
}

void LED_IO_Complete(void)
{
    osSignalSet(status_led, 0);
}

static void RGB_Thread(const void *args)
{
    (void)args;
    osDelay(10);
    while(1)
    {
        LED(0, 255, 0, 0);
        osDelay(200);
        LED(1, 0, 255, 0);
        osDelay(200);
        LED(2, 0, 0, 255);
        LED(0, 0, 255, 0);
        osDelay(200);
        LED(1, 0, 0, 255);
        osDelay(200);
        LED(2, 255, 0, 0);
        LED(0, 0, 0, 255);
        osDelay(200);
        LED(1, 255, 0, 0);
        osDelay(200);
        LED(2, 0, 255, 0);
        LED(0, 0, 0, 0);
        osDelay(200);
        LED(1, 0, 0, 0);
        osDelay(200);
        LED(2, 0, 0, 0);
        osDelay(200);
       }
}

void LED_TestPatternStart(void)
{
  rgb_tid = osThreadCreate(osThread(rgb), NULL);
}

void LED_TestPatternStop(void)
{
  osThreadTerminate(rgb_tid);
}

void LED_R(uint8_t idx, uint8_t r)
{
    osMessagePut(commandQ, COMMAND_S(idx * 3, COMMAND_CHANNEL_R, r), 0);
}

void LED_G(uint8_t idx, uint8_t g)
{
    osMessagePut(commandQ, COMMAND_S(idx * 3, COMMAND_CHANNEL_G, g), 0);
}

void LED_B(uint8_t idx, uint8_t b)
{
    osMessagePut(commandQ, COMMAND_S(idx * 3, COMMAND_CHANNEL_B, b), 0);
}

void LED_Raw(uint8_t idx, uint8_t v)
{
    osMessagePut(commandQ, COMMAND_RAW(idx, v), 0);
}

