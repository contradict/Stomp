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

//  BB MJS: removed hold_down
//  BB MJS: What about selfright?  Simply fire hammer, right?
//  BB MJS: Self right is just fire hammer.  So removed that all
//  BB MJS: Big thing is that turret is no longer deailing with the wheel drive motors
//          but we will have one drive motor for turret control.
//  BB MJS: So for now, removed dribe.h and autodrive.h.  Use as reference for how to control new motor
//  BB MJS: Also removed rc_pwm because we are going to drive the motor differently

uint32_t start_time, loop_speed_min, loop_speed_avg, loop_speed_max, loop_count;
void reset_loop_stats(void) {
    loop_count = loop_speed_max = loop_speed_avg = 0;
    loop_speed_min = (uint32_t)(-1L);
}

void update_loop_stats() {
    uint32_t loop_speed = micros() - start_time;
    start_time = micros();
    loop_speed_min = min(loop_speed, loop_speed_min);
    loop_speed_avg += loop_speed;
    loop_count += 1;
    loop_speed_max = max(loop_speed, loop_speed_max);
}

static uint32_t last_request_time = micros();
static uint32_t last_sensor_time = micros();
static int16_t steer_bias = 0; // positive turns left, negative turns right
static enum AutofireState autofire = AF_NO_TARGET;

extern uint16_t leddar_overrun;
extern uint16_t leddar_crc_error;
extern uint16_t sbus_overrun;
extern uint8_t HAMMER_INTENSITIES_ANGLE[9];

uint32_t sensor_period=5000L;
uint32_t leddar_max_request_period=100000L;

// parameters written in command
Track tracked_object;

void turretSetup() {
    // Come up safely
    safeState();
    wdt_enable(WDTO_4S);
    xbeeInit();
    SBusInit();
    leddarWrapperInit();
    driveSetup();
    sensorSetup();
    initializeIMU();
    reset_loop_stats();
    restoreObjectSegmentationParameters();
    tracked_object.restoreTrackingFilterParams();
    restoreAutofireParameters();
    restoreTelemetryParameters();
    debug_print("STARTUP");
    start_time = micros();
}

void turretLoop() {
    if(micros() - last_sensor_time>sensor_period) {
        readSensors();
        last_sensor_time = micros();
    }

    // check for data from weapons radio
    bool working = sbusGood();
    if (working) {
        wdt_reset();
    }
    uint16_t current_rc_bitfield = getRcBitfield();

    // If there has been no data from the LEDDAR for a while, ask again
    if (micros() - last_request_time > leddar_max_request_period){
        last_request_time = micros();
        requestDetections();
    }

    int16_t hammer_intensity = getHammerIntensity();
    int16_t hammer_distance = getRange();
    int16_t turret_speed = getTurretSpeed();

    if (working)
    {
        drive(turret_speed);
    }

    // Check if data is available from the LEDDAR
    if (bufferDetections()){

        uint32_t now = micros();
        // extract detections from LEDDAR packet
        uint8_t raw_detection_count = parseDetections();

        // request new detections
        last_request_time = micros();
        requestDetections();

        calculateMinimumDetections(raw_detection_count);
        const Detection (*minDetections)[LEDDAR_SEGMENTS] = NULL;
        getMinimumDetections(&minDetections);

        Object objects[8];
        uint8_t num_objects = segmentObjects(*minDetections, now, objects);

        int8_t best_object = trackObject(now, objects, num_objects, tracked_object);

        // auto centering code
        //  BB MJS: Commented out for now
        // new_autodrive = pidSteer(tracked_object, drive_range, &drive_bias, &steer_bias);

        autofire = willHit(tracked_object, hammer_distance, hammer_intensity);
        if((autofire==AF_HIT) && (current_rc_bitfield & AUTO_HAMMER_ENABLE_BIT)) {
            fire(hammer_intensity, current_rc_bitfield & FLAME_PULSE_BIT, true);
        }

        // Send subsampled leddar telem
        if (isTimeToSendLeddarTelem(now)){
            sendLeddarTelem(*minDetections, raw_detection_count);
            sendObjectsTelemetry(num_objects, objects);

            if(num_objects > 0)
            {
                sendTrackingTelemetry(
                        objects[best_object].xcoord(), objects[best_object].ycoord(),
                        objects[best_object].angle(), objects[best_object].radius(),
                        tracked_object.x/16, tracked_object.vx/16,
                        tracked_object.y/16, tracked_object.vy/16);
            }
            else
            {
                sendTrackingTelemetry(
                        0, 0, 0, 0,
                        tracked_object.x/16, tracked_object.vx/16,
                        tracked_object.y/16, tracked_object.vy/16);
            }
        }
    }

    // React to RC state changes (change since last time this call was made)
    int16_t diff = getRcBitfieldChanges();
    if( !(current_rc_bitfield & FLAME_PULSE_BIT) && !(current_rc_bitfield & FLAME_CTRL_BIT) ){
        flameSafe();
    }
    if( (diff & FLAME_PULSE_BIT) && (current_rc_bitfield & FLAME_PULSE_BIT) ){
        flameEnable();
    }
    // Flame on -> off
    if( (diff & FLAME_CTRL_BIT) && !(current_rc_bitfield & FLAME_CTRL_BIT) ){
        flameEnd();
    }
    // Flame off -> on
    if( (diff & FLAME_CTRL_BIT) && (current_rc_bitfield & FLAME_CTRL_BIT) ){
        flameEnable();
        flameStart();
    }
    // Manual hammer fire
    if( (diff & HAMMER_FIRE_BIT) && (current_rc_bitfield & HAMMER_FIRE_BIT)){
        if (current_rc_bitfield & DANGER_CTRL_BIT){
          noAngleFire(hammer_intensity, current_rc_bitfield & FLAME_PULSE_BIT);
        } else {
          fire(hammer_intensity, current_rc_bitfield & FLAME_PULSE_BIT, false /*autofire*/);
        }
    }

    //  BB MJS: Hammer control is going to be different this year.  There is no motor drive, just to Big
    //  cylindars for hard hit and gentral retract

    if( (diff & HAMMER_RETRACT_BIT) && (current_rc_bitfield & HAMMER_RETRACT_BIT)){
      if (current_rc_bitfield & DANGER_CTRL_BIT){
        gentleRetract(HAMMER_RETRACT_BIT);
      } else {
        retract();
      }
    }
    if( (current_rc_bitfield & GENTLE_HAM_F_BIT)) {
        gentleFire(GENTLE_HAM_F_BIT);
    }
    if( (current_rc_bitfield & GENTLE_HAM_R_BIT)) {
        gentleRetract(GENTLE_HAM_R_BIT);
    }

    // read IMU and compute orientation
    processIMU();

    // send telemetry
    uint32_t now = micros();
    if (isTimeToSendTelemetry(now)) {
        // get targeting RC command.
        sendSensorTelem(getPressure(), getAngle());
        sendSystemTelem(loop_speed_min, loop_speed_avg/loop_count,
                        loop_speed_max, loop_count,
                        leddar_overrun,
                        leddar_crc_error,
                        sbus_overrun,
                        last_command,
                        command_overrun,
                        invalid_command,
                        valid_command);
        reset_loop_stats();
        int16_t hammer_angle = HAMMER_INTENSITIES_ANGLE[hammer_intensity];
        sendSbusTelem(current_rc_bitfield, hammer_angle, hammer_distance);
        telemetryIMU();
    }

    handle_commands();
    update_loop_stats();
}
