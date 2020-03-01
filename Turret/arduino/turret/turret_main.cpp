#include "Arduino.h"
#include "turret_main.h"
#include "sbus.h"
#include "leddar_io.h"
#include "sensors.h"
#include "drive.h"
#include "xbee.h"
#include "telem.h"
#include "pins.h"
#include "weapons.h"
#include "utils.h"
#include "targeting.h"
#include <avr/wdt.h>
#include "command.h"
#include "imu.h"
#include "autofire.h"
#include "autoaim.h"

#include "turretController.h"

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

static TurretController s_turretController;

//  Keep track of radio communications status and state

static int16_t s_hammerIntensity;
static int16_t s_hammerDistance;
static uint16_t s_currentRCBitfield;

//  Have various update frequencies for different systems

static uint32_t s_last_request_time = micros();
static uint32_t s_last_sensor_time = micros();

static enum AutoFireState s_autoFire = AF_NO_TARGET;
static enum AutoAimState s_autoAim = AA_NO_TARGET;

static int16_t s_steer_bias = 0; // positive turns left, negative turns right


//  ====================================================================
//
//  External references
//
//  ====================================================================

extern uint16_t leddar_overrun;
extern uint16_t leddar_crc_error;
extern uint16_t sbus_overrun;
extern uint8_t HAMMER_INTENSITIES_ANGLE[9];

//  ====================================================================
//
//  Forward references to internal (private) methods
//
//  ====================================================================

static void sendTelemetry(void);
static void updateTurretRotation(void);
static void updateWeapons(void);
static void updateTracking(void);
static void updateWatchDogTimer(void);
static void updateSensors(void);
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

void turretSetup() 
{
    //  Make sure we come up safly and enable watch dog to 
    //  ensure if we don't have communications, we reset.

    safeState();
    wdt_enable(WDTO_4S);

    //  Initialize all of the various subsystems
    //  to initial state
    
    initXbee();
    initSBus();
    initLeddarWrapper();
    initSensors();
    initIMU();

    s_turretController.Init();

    //  Restore any information stored in EEMEM

    restoreObjectSegmentationParameters();
    restoreAutoFireParameters();
    restoreAutoAimParameters();
    restoreTelemetryParameters();

    g_trackedObject.restoreTrackingFilterParams();
    s_turretController.RestoreParams();

    // Turret init

    reset_loop_stats();
    g_start_time = micros();

    debug_print("STARTUP");
}

//
//  Main update loop
//

void turretLoop() 
{
    //  First, update raido connection state to see if everything is good
    //  If everything is good, will also update information about controller 
    //  state, such as the requested hammer instnsity, drive speed, etc.

    updateRadio();

    //  Next, do general systems updates.
    //  Reset watch dog timmer if we have a connection and give the
    //  the sensors their update tick
    
    updateWatchDogTimer();
    updateSensors();
    updateIMU();

    //  Update Tracking and auto fire using Lidar

    updateTracking();

    //  Update Weapons
    //  Take care of hammer and flame thrower state
    //  This includes manually firing the hammer or flame throwers

    updateWeapons();

    //  Calculate and apply turret rotation.  Accounts for systems adjustments
    //  such as auto aim as well as requested adjustmens from the radio controller

    s_turretController.Update();

    //  Send out telemetry to PC and process commands from PC

    sendTelemetry();
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

static void updateSensors()
{
    //  Update the sensor data

    if(micros() - s_last_sensor_time > k_sensor_period) 
    {
        readSensors();
        s_last_sensor_time = micros();
    }
}

static void updateWatchDogTimer()
{
    //  As long as we have valid radio connection,
    //  prevent watch dog timer from expiring

    if (true) // isRadioConnected()) 
    {
        wdt_reset();
    }
}

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

    //  Get the state of autoAim and autoFire

    s_autoFire = updateAutoFire(g_trackedObject, s_hammerDistance, s_hammerIntensity);
    s_autoAim = updateAutoAim(g_trackedObject);

    if((s_autoFire==AF_HIT) && (s_currentRCBitfield & AUTO_HAMMER_ENABLE_BIT)) 
    {
        fire(s_hammerIntensity, s_currentRCBitfield & FLAME_PULSE_BIT, true);
    }

    if((s_autoAim == AA_LOCKED_ON) && (s_currentRCBitfield & AUTO_AIM_ENABLE_BIT)) 
    {
    }


    //  Send out tracking / auto aim telemetry
  
    if (isTimeToSendLeddarTelem(now))
    {
        sendLeddarTelem(*minDetections, raw_detection_count);
        sendObjectsTelemetry(num_objects, objects);

        if(num_objects > 0)
        {
            sendTrackingTelemetry(
                    objects[best_object].xcoord(), objects[best_object].ycoord(),
                    objects[best_object].angle(), objects[best_object].radius(),
                    g_trackedObject.x/16, g_trackedObject.vx/16,
                    g_trackedObject.y/16, g_trackedObject.vy/16);
        }
        else
        {
            sendTrackingTelemetry(
                    0, 0, 0, 0,
                    g_trackedObject.x/16, g_trackedObject.vx/16,
                    g_trackedObject.y/16, g_trackedObject.vy/16);
        }
    }
}

static void updateWeapons()
{
    // React to RC state changes (change since last time this call was made)
    int16_t diff = getRcBitfieldChanges();
    
    if( !(s_currentRCBitfield & FLAME_PULSE_BIT) && !(s_currentRCBitfield & FLAME_CTRL_BIT) ){
        flameSafe();
    }
    if( (diff & FLAME_PULSE_BIT) && (s_currentRCBitfield & FLAME_PULSE_BIT) ){
        flameEnable();
    }
    // Flame on -> off
    if( (diff & FLAME_CTRL_BIT) && !(s_currentRCBitfield & FLAME_CTRL_BIT) ){
        flameEnd();
    }
    // Flame off -> on
    if( (diff & FLAME_CTRL_BIT) && (s_currentRCBitfield & FLAME_CTRL_BIT) ){
        flameEnable();
        flameStart();
    }
    // Manual hammer fire
    if( (diff & HAMMER_FIRE_BIT) && (s_currentRCBitfield & HAMMER_FIRE_BIT)){
        if (s_currentRCBitfield & DANGER_CTRL_BIT){
          noAngleFire(s_hammerIntensity, s_currentRCBitfield & FLAME_PULSE_BIT);
        } else {
          fire(s_hammerIntensity, s_currentRCBitfield & FLAME_PULSE_BIT, false /*autofire*/);
        }
    }

    //  BB MJS: Hammer control is going to be different this year.  There is no motor drive, just to Big
    //  cylindars for hard hit and gentral retract

    if( (diff & HAMMER_RETRACT_BIT) && (s_currentRCBitfield & HAMMER_RETRACT_BIT)){
      if (s_currentRCBitfield & DANGER_CTRL_BIT){
        gentleRetract(HAMMER_RETRACT_BIT);
      } else {
        retract();
      }
    }
    if( (s_currentRCBitfield & GENTLE_HAM_F_BIT)) {
        gentleFire(GENTLE_HAM_F_BIT);
    }
    if( (s_currentRCBitfield & GENTLE_HAM_R_BIT)) {
        gentleRetract(GENTLE_HAM_R_BIT);
    }
}

static void sendTelemetry()
{
    // send telemetry
    uint32_t now = micros();
 
    if (!isTimeToSendTelemetry(now)) 
    {
        return;
    }

    s_turretController.SendTelem();

    sendSensorTelem(getPressure(), getAngle());

    sendSystemTelem(g_loop_speed_min, g_loop_speed_avg/g_loop_count,
                    g_loop_speed_max, g_loop_count,
                    leddar_overrun,
                    leddar_crc_error,
                    sbus_overrun,
                    last_command,
                    command_overrun,
                    invalid_command,
                    valid_command);

    reset_loop_stats();
    int16_t hammer_angle = HAMMER_INTENSITIES_ANGLE[s_hammerIntensity];

    sendSbusTelem(s_currentRCBitfield, hammer_angle, s_hammerDistance);

    telemetryIMU();
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

