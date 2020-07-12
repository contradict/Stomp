#include "Arduino.h"
#include "sbus.h"
#include "leddar_io.h"
#include "pins.h"
#include "utils.h"
#include "targeting.h"
#include <avr/wdt.h>
#include "command.h"
#include "imu.h"
#include "autofire.h"
#include "autoaim.h"

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

//  Controller Objects 

Track g_trackedObject;

//  ====================================================================
//
//  Global constants
//
//  ====================================================================

const uint32_t k_sensor_period=5000L;
const uint32_t k_leddar_max_request_period=100000L;

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

static void updateTracking(void);
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
    initIMU();

    //  Restore any information stored in EEMEM

    restoreObjectSegmentationParameters();


    g_trackedObject.restoreTrackingFilterParams();

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
    updateIMU();

    //  Update Tracking and auto fire using Lidar

    updateTracking();
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

// BB MJS Move Update Tracking and SendLeddarTelem to autoaim.cpp

static void updateTracking()
{
    // If there has been no data from the LEDDAR for a while, ask again

    if (micros() - s_last_request_time > k_leddar_max_request_period)
    {
        s_last_request_time = micros();
        requestDetections();
    }

    //  Check if data is available from the LEDDAR
    //  If not, just bail 

    if (!bufferDetections())
    {
        return;
    }

    uint32_t now = micros();

    // extract detections from LEDDAR packet
    uint8_t raw_detection_count = parseDetections();

    // request new detections
    s_last_request_time = micros();
    requestDetections();

    calculateMinimumDetections(raw_detection_count);

    const Detection (*minDetections)[LEDDAR_SEGMENTS] = NULL;
    getMinimumDetections(&minDetections);

    Object objects[8];
    uint8_t num_objects = segmentObjects(*minDetections, now, objects);
    int8_t best_object = trackObject(now, objects, num_objects, g_trackedObject);
}

void turretSendLeddarTelem()
{
/*
    //  Send out tracking / auto aim telemetry

    Telem.SendLeddarTelem(*minDetections, raw_detection_count);
    Telem.SendObjectsMeasuredTelemetry(num_objects, objects);
    Telem.SendObjectsCalculatedTelemetry(num_objects, objects);


    if(num_objects > 0)
    {
        Telem.SendTrackingTelemetry(
                objects[best_object].xcoord(), objects[best_object].ycoord(),
                objects[best_object].angle(), objects[best_object].radius(),
                g_trackedObject.x/16, g_trackedObject.vx/16,
                g_trackedObject.y/16, g_trackedObject.vy/16);
    }
    else
    {
        Telem.SendTrackingTelemetry(
                0, 0, 0, 0,
                g_trackedObject.x/16, g_trackedObject.vx/16,
                g_trackedObject.y/16, g_trackedObject.vy/16);
    }
*/
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

    Telem.SendSbusTelem(s_currentRCBitfield, hammer_angle, s_hammerDistance, Turret.GetTurretSpeed());
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

