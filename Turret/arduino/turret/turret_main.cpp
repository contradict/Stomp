#include "Arduino.h"
#include "sbus.h"
#include "leddar_io.h"
#include "pins.h"
#include "utils.h"
#include <avr/wdt.h>
#include "command.h"
#include "autoAimController.h"

#include "radioController.h"
#include "telemetryController.h"
#include "turretController.h"

#include "turret_main.h"

//  ====================================================================
//
//  Global Variables
//
//  ====================================================================

//  deltaTime managment

uint32_t g_dT = 0;
uint32_t g_start_time;

//  update loop stats

uint32_t g_loop_speed_min;
uint32_t g_loop_speed_avg; 
uint32_t g_loop_speed_max; 
uint32_t g_loop_count;

//  ====================================================================
//
//  Global constants
//
//  ====================================================================

const uint32_t k_sensor_period=5000L;

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

//  Keep track of radio communications status and state

static int16_t s_hammerIntensity;
static int16_t s_hammerDistance;
static uint16_t s_currentRCBitfield;

//  Have various update frequencies for different systems

static uint32_t s_last_request_time = micros();
static uint32_t s_last_sensor_time = micros();

//  ====================================================================
//
//  External references
//
//  ====================================================================

extern uint16_t leddar_overrun;
extern uint16_t leddar_crc_error;
extern uint16_t sbus_overrun;

//  ====================================================================
//
//  Forward references to internal (private) methods
//
//  ====================================================================

static void updateWatchDogTimer(void);
static void updateRadio(void);

static void reset_loop_stats(void);
static void updateLoopStats(void);

//  ====================================================================
//
//  Global (public) methods
//
//  ====================================================================

//  
//  Main initialization point
//

void turretInit()
{
    //  Make sure we come up safly and enable watch dog to 
    //  ensure if we don't have communications, we reset.

    // BB MJS: safeState();
    wdt_enable(WDTO_4S);

    //  Initialize all of the various subsystems
    //  to initial state

    initSBus();
    initLeddarWrapper();

    reset_loop_stats();
    g_start_time = micros();
}

//
//  Main update loop
//

void turretUpdate() 
{
    //  First, update radio connection state to see if everything is good
    //  If everything is good, will also update information about controller 
    //  state, such as the requested hammer instnsity, drive speed, etc.

    updateRadio();

    //  Next, do general systems updates.
    //  Reset watch dog timmer if we have a connection and give the
    //  the sensors their update tick
    
    updateWatchDogTimer();

    handleCommands();

    // Finally update our loop stats
    updateLoopStats();
}

//  ====================================================================
//
//  Internal (private) Methods
//
//  ====================================================================

static void updateRadio()
{
    updateSBus();

    s_currentRCBitfield = getRcBitfield();
    s_hammerIntensity = getHammerIntensity();
    s_hammerDistance = getRange();
}

static void updateWatchDogTimer()
{
    //  As long as we have valid radio connection,
    //  prevent watch dog timer from expiring

    if (Radio.IsNominal())
    {
        wdt_reset();
    }
}

void turretSendTelem()
{
    Telem.SendSystemTelem(g_loop_speed_min, g_loop_speed_avg/g_loop_count,
                    g_loop_speed_max, g_loop_count,
                    leddar_overrun,
                    leddar_crc_error,
                    sbus_overrun,
                    last_command,
                    command_overrun,
                    invalid_command,
                    valid_command);

    reset_loop_stats();
    int16_t hammer_angle = 0; // BB MJS -> HAMMER_INTENSITIES_ANGLE[s_hammerIntensity];

    Telem.SendSbusTelem(s_currentRCBitfield, hammer_angle, s_hammerDistance, Turret.GetTurretRotationSpeed());
}

static void reset_loop_stats(void) 
{
    g_loop_count = g_loop_speed_max = g_loop_speed_avg = 0;
    g_loop_speed_min = (uint32_t)(-1L);
}

static void updateLoopStats(void) 
{
    uint32_t loop_speed = micros() - g_start_time;
    g_start_time = micros();

    g_loop_speed_min = min(loop_speed, g_loop_speed_min);
    g_loop_speed_avg += loop_speed;
    g_loop_count += 1;
    g_loop_speed_max = max(loop_speed, g_loop_speed_max);
}

