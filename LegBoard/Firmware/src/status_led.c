#include "cmsis_os.h"
#include "is31fl3235.h"

#define COMMAND_IDX(value) ((value>>24) & 0xFF)
#define COMMAND_R(value) ((value>>16) & 0xFF)
#define COMMAND_G(value) ((value>>8) & 0xFF)
#define COMMAND_B(value) (value & 0xFF)

#define COMMAND(idx, r, g, b) (((idx&0xff) << 24) | ((r&0xff)<<16) | ((g&0xff)<<8) | (b&0xff))

static void LED_Thread(const void *args);

osThreadDef(led, LED_Thread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
osMessageQDef(ledcommand, 32, uint32_t);
static osThreadId status_led;
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
    uint8_t idx, rgb[3];
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
        rgb[0] = COMMAND_R(event.value.v);
        rgb[1] = COMMAND_G(event.value.v);
        rgb[2] = COMMAND_B(event.value.v);
        while(1)
        {
            is31fl3235_Set(idx, 3, rgb);
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
    osMessagePut(commandQ, COMMAND(idx * 3, r, g, b), 0);
}

void LED_IO_Complete(void)
{
    osSignalSet(status_led, 0);
}
