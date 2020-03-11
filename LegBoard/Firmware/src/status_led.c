#include <string.h>
#include "cmsis_os.h"
#include "timers.h"
#include "is31fl3235.h"

static void LED_Thread(const void *args);
static void RGB_Thread(const void *args);

osThreadDef(led, LED_Thread, osPriorityIdle, 1, configMINIMAL_STACK_SIZE);
osThreadDef(rgb, RGB_Thread, osPriorityIdle, 1, configMINIMAL_STACK_SIZE);

enum StatusLEDCommand
{
    SET,
    BLINK,
    BLINK_OVER
};

struct StatusLEDMessage {
    uint8_t command, start, count, duration;
    uint8_t values[4];
    osTimerId timer;
};

osMailQDef(ledcommand, 32, struct StatusLEDMessage);

static osThreadId status_led;
static osThreadId rgb_tid;

static osMailQId commandQ;

static uint8_t zeros[4] = {0,0,0,0};

static bool blink_in_progress[32];

void LED_ThreadInit(void)
{
    bzero(blink_in_progress, sizeof(blink_in_progress));
    is31fl3235_Init(IS31FL3235_ADDR_GND);

    status_led = osThreadCreate(osThread(led), NULL);
    commandQ = osMailCreate(osMailQ(ledcommand), status_led);
}

static void LED_BlinkOver(void const *arg)
{
    struct StatusLEDMessage *msg = (struct StatusLEDMessage *)pvTimerGetTimerID((TimerHandle_t)arg);
    msg->command = BLINK_OVER;
    bzero(msg->values, msg->count);
    osMailPut(commandQ, msg);
}

static void LED_Set(struct StatusLEDMessage *msg)
{
    osEvent event;
    while(1)
    {
        is31fl3235_Set(msg->start, msg->count, msg->values);
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

static void LED_Thread(const void *args)
{
    (void)args;
    osEvent event;
    uint8_t control_default[9] = {
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX4 | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX4 | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX | IS31FL3235_LED_CONTROL_OUT_ON,
        IS31FL3235_LED_CONTROL_CURRENT_IMAX4 | IS31FL3235_LED_CONTROL_OUT_ON
    };

    osTimerDef(blink_timer, LED_BlinkOver);

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
        event = osMailGet(commandQ, osWaitForever);
        if(event.status != osEventMail)
        {
            continue;
        }
        struct StatusLEDMessage *msg = (struct StatusLEDMessage *)event.value.p;
        LED_Set(msg);
        switch((enum StatusLEDCommand)msg->command)
        {
            case SET:
                osMailFree(commandQ, msg);
                break;
            case BLINK:
                for(int i=msg->start; i<msg->start + msg->count; i++)
                    blink_in_progress[i] = true;
                msg->timer = osTimerCreate(osTimer(blink_timer), osTimerOnce, msg);
                osTimerStart(msg->timer, msg->duration);
                break;
            case BLINK_OVER:
                for(int i=msg->start; i<msg->start + msg->count; i++)
                    blink_in_progress[i] = false;
                osTimerDelete(msg->timer);
                osMailFree(commandQ, msg);
                break;
        }
    }
}

void LED_SetRGB(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
    struct StatusLEDMessage *msg = osMailAlloc(commandQ, 0);
    if(msg == NULL)
    {
        return;
    }
    msg->command = SET;
    msg->start = 3*idx;
    msg->count = 3;
    msg->values[0] = r;
    msg->values[1] = g;
    msg->values[2] = b;
    osMailPut(commandQ, msg);
}

void LED_SetOne(uint8_t idx, uint8_t channel, uint8_t r)
{
    struct StatusLEDMessage *msg = osMailAlloc(commandQ, 0);
    if(msg == NULL)
    {
        return;
    }
    msg->command = SET;
    msg->start = 3*idx + channel;
    msg->count = 1;
    msg->values[0] = r;
    osMailPut(commandQ, msg);
}

bool LED_IsBlinking(struct StatusLEDMessage *msg)
{
    bool in_progress = false;
    for(int i=msg->start; i<msg->start + msg->count; i++)
        in_progress |= blink_in_progress[i];
    return in_progress;    
}

void LED_BlinkRGB(uint8_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t duration)
{
    struct StatusLEDMessage *msg = osMailAlloc(commandQ, 0);
    if(msg == NULL)
    {
        return;
    }
    msg->command = BLINK;
    msg->duration = duration;
    msg->start = 3*idx;
    msg->count = 3;
    msg->values[0] = r;
    msg->values[1] = g;
    msg->values[2] = b;
    if(!LED_IsBlinking(msg))
    {
        osMailPut(commandQ, msg);
    }
    else
    {
        osMailFree(commandQ, msg);
    }
}

void LED_BlinkOne(uint8_t idx, uint8_t channel, uint8_t r, uint8_t duration)
{
    struct StatusLEDMessage *msg = osMailAlloc(commandQ, 0);
    if(msg == NULL)
    {
        return;
    }
    msg->command = BLINK;
    msg->duration = duration;
    msg->start = 3*idx + channel;
    msg->count = 1;
    msg->values[0] = r;
    if(!LED_IsBlinking(msg))
    {
        osMailPut(commandQ, msg);
    }
    else
    {
        osMailFree(commandQ, msg);
    }
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
        LED_SetRGB(0, 255, 0, 0);
        osDelay(200);
        LED_SetRGB(1, 0, 255, 0);
        osDelay(200);
        LED_SetRGB(2, 0, 0, 255);
        LED_SetRGB(0, 0, 255, 0);
        osDelay(200);
        LED_SetRGB(1, 0, 0, 255);
        osDelay(200);
        LED_SetRGB(2, 255, 0, 0);
        LED_SetRGB(0, 0, 0, 255);
        osDelay(200);
        LED_SetRGB(1, 255, 0, 0);
        osDelay(200);
        LED_SetRGB(2, 0, 255, 0);
        LED_SetRGB(0, 0, 0, 0);
        osDelay(200);
        LED_SetRGB(1, 0, 0, 0);
        osDelay(200);
        LED_SetRGB(2, 0, 0, 0);
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


