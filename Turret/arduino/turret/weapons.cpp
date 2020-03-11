#include "Arduino.h"
#include "weapons.h"
#include "sensors.h"
#include "pins.h"
#include "utils.h"
#include "telem.h"
#include <avr/wdt.h>

extern HardwareSerial& TurretRotationMotorSerial;

uint8_t MAX_SAFE_ANGLE = 65;
uint8_t HAMMER_INTENSITIES_ANGLE[9] = { 3, 5, 10, 15, 20, 30, 40, 50, 65 };

uint8_t MAX_SAFE_TIME = 125;
//                                     3   5   10  15  20  30  40  50  65
uint8_t HAMMER_INTENSITIES_TIME[9] = { 25, 35, 60, 70, 80, 95, 105, 115, 125 };

bool weaponsEnabled(){
    return g_enabled;
}

// hammer moving with electric motor
static bool hammer_in_motion = false;

// HAMMER DATA BUFFERS
#define MAX_DATAPOINTS 500
static uint16_t angle_data[MAX_DATAPOINTS];
static int16_t pressure_data[MAX_DATAPOINTS];

// HAMMER THROW CONSTANTS
// #define THROW_CLOSE_ANGLE_DIFF 3  // angle distance between throw open and throw close
#define VENT_OPEN_ANGLE 175
#define DATA_COLLECT_TIMESTEP 2000  // timestep for data logging, in microseconds
static const uint32_t SWING_TIMEOUT = 1500 * 1000L;  // in microseconds
static const uint16_t THROW_BEGIN_ANGLE_MIN = RELATIVE_TO_BACK - 5;
static const uint16_t THROW_BEGIN_ANGLE_MAX = RELATIVE_TO_BACK + 10;
static const uint16_t THROW_COMPLETE_ANGLE = RELATIVE_TO_FORWARD;
#define AUTO_RETRACT_MIN_ANGLE 160

void retract( bool check_velocity ){
    uint16_t angle;
    uint32_t sensor_read_time;
    uint32_t delay_time;
    uint32_t retract_time;

    bool velocity_ok = true;
    if (check_velocity){
      float angular_velocity;
      bool velocity_read_ok = angularVelocity(&angular_velocity);
      velocity_ok = velocity_read_ok && abs(angular_velocity) < RETRACT_BEGIN_VEL_MAX;
    }

    bool angle_read_ok = readAngle(&angle);
    // Only retract if hammer is forward and not moving
    if (weaponsEnabled() && angle_read_ok && angle > RETRACT_COMPLETE_ANGLE && velocity_ok) {
        retract_time = micros();
        String movecmd = startElectricHammerMove(1000);
        while (micros() - retract_time < RETRACT_TIMEOUT && angle > RETRACT_COMPLETE_ANGLE) {
            // process sbus data and check if weaponsEnabled has changed state
            updateSBus();
            sensor_read_time = micros();
            readAngle(&angle);
            // Ensure that loop step takes 1 ms or more (without this it takes quite a bit less)
            sensor_read_time = micros() - sensor_read_time;
            delay_time = 10000 - sensor_read_time;
            if (delay_time > 0) {
                delayMicroseconds(delay_time);
            }
            TurretRotationMotorSerial.println(movecmd);
        }
        stopElectricHammerMove();
    }
}

// Helper to end a swing in case of timeout or hammer obstruction (zero velocity)
void endSwing( bool& throw_open, bool& vent_closed, uint16_t& throw_close_timestep, uint16_t& vent_open_timestep, uint16_t timestep){
  if (throw_open) {
    throw_close_timestep = timestep;
  }
  safeDigitalWrite(THROW_VALVE_DO, LOW);
  throw_open = false;
  delay(10);
  if (vent_closed) {
    vent_open_timestep = timestep;
  }
  safeDigitalWrite(VENT_VALVE_DO, LOW);
  vent_closed = false;
}

void fire( uint16_t hammer_intensity, bool flame_pulse, bool autofire ){
    uint32_t fire_time;
    uint32_t swing_length = 0;
    uint32_t sensor_read_time;
    uint16_t throw_close_timestep = 0;
    uint16_t vent_open_timestep = 0;
    uint16_t datapoints_collected = 0;
    uint16_t timestep = 0;
    bool vent_closed = false;
    bool throw_open = false;
    uint32_t delay_time;
    uint16_t angle;
    uint16_t start_angle;
    int16_t pressure;
    bool pressure_read_ok;

    bool angle_read_ok = readAngle(&angle);
    if (weaponsEnabled() && angle_read_ok && !hammer_in_motion){
        start_angle = angle;
        // Just in case a bug causes us to fall out of the hammer intensities array, do a last minute
        // sanity check before we actually command a throw.
        uint16_t throw_close_angle_diff = min(MAX_SAFE_ANGLE, HAMMER_INTENSITIES_ANGLE[hammer_intensity]);
        uint16_t throw_close_angle = start_angle + throw_close_angle_diff;
        if (angle > THROW_BEGIN_ANGLE_MIN && angle < THROW_BEGIN_ANGLE_MAX) {
 
            if (flame_pulse){
                flameStart();
            }

            // Seal vent (which is normally open)
            safeDigitalWrite(VENT_VALVE_DO, HIGH);
            vent_closed = true;
            // can we actually determine vent close time?
            delay(10);

            // Open throw valve
            safeDigitalWrite(THROW_VALVE_DO, HIGH);
            throw_open = true;
            fire_time = micros();
            // Wait until hammer swing complete, up to timeout
            while (swing_length < SWING_TIMEOUT) {
                sensor_read_time = micros();
                angle_read_ok = readAngle(&angle);
                pressure_read_ok = readMlhPressure(&pressure);

                if (throw_open && angle > throw_close_angle) {
                    throw_close_timestep = timestep;
                    safeDigitalWrite(THROW_VALVE_DO, LOW);
                    throw_open = false;
                }
                if (vent_closed && angle > VENT_OPEN_ANGLE) {
                    vent_open_timestep = timestep;
                    safeDigitalWrite(VENT_VALVE_DO, LOW);
                    vent_closed = false;
                }
                if (datapoints_collected < MAX_DATAPOINTS){
                    angle_data[datapoints_collected] = angle;
                    if(pressure_read_ok) {
                        pressure_data[datapoints_collected] = pressure;
                    } else {
                        pressure_data[datapoints_collected] = -1;
                    }
                    datapoints_collected++;
                }

                // Once past our throw close angle, start checking velocity 
                if (angle > AUTO_RETRACT_MIN_ANGLE) {
                    float angular_velocity;
                    bool velocity_read_ok = angularVelocityBuffered(&angular_velocity, angle_data, datapoints_collected, DATA_COLLECT_TIMESTEP/1000);
                    if (velocity_read_ok && abs(angular_velocity) < RETRACT_BEGIN_VEL_MAX) {
                        // If the swing hasn't already ended, end it now
                        endSwing(throw_open, vent_closed, throw_close_timestep, vent_open_timestep, timestep);
                        // Since our final velocity is low enough, auto-retract
                        retract( /*check_velocity*/ false );
                        break; // exit the while loop
                    }
                }

                // Ensure that loop step takes 1 ms or more (without this it takes quite a bit less)
                sensor_read_time = micros() - sensor_read_time;
                delay_time = DATA_COLLECT_TIMESTEP - sensor_read_time;
                if (delay_time > 0) {
                    delayMicroseconds(delay_time);
                }
                timestep++;
                swing_length = micros() - fire_time;
            } // while
            // If the swing hasn't already ended, end it now
            endSwing(throw_open, vent_closed, throw_close_timestep, vent_open_timestep, timestep);

            if (flame_pulse) {
                flameEnd();
            }
        } else if (!autofire) {
            // If we're *not* in autochomp mode, and the hammer is at a funny angle, it probably
            // means we're in a weird spot and maybe want to unstick ourselves with a
            // minimum-intensity danger fire.
            noAngleFire(/* hammer intensity */1, false);
            return;
        }

        sendSwingTelem(datapoints_collected,
                      angle_data,
                      pressure_data,
                      DATA_COLLECT_TIMESTEP,
                      throw_close_timestep,
                      vent_open_timestep,
                      throw_close_angle,
                      start_angle);
    }
}

const uint8_t NO_ANGLE_SWING_DURATION = 185; // total estimated time in ms of a swing (to calculate vent time)
void noAngleFire( uint16_t hammer_intensity, bool flame_pulse){
    if (weaponsEnabled() && !hammer_in_motion){
        if (flame_pulse){
            flameStart();
        }
        uint8_t throw_duration = min(MAX_SAFE_TIME, HAMMER_INTENSITIES_TIME[hammer_intensity]);
        // Seal vent valve
        safeDigitalWrite(VENT_VALVE_DO, HIGH);
        // can we actually determine vent close time?
        delay(10);
        
        // Open throw valve
        safeDigitalWrite(THROW_VALVE_DO, HIGH);
        delay(throw_duration);
        safeDigitalWrite(THROW_VALVE_DO, LOW);

        // Wait the estimated remaining time in the swing and then vent
        delay(NO_ANGLE_SWING_DURATION - throw_duration);
        safeDigitalWrite(VENT_VALVE_DO, LOW);

        if (flame_pulse) {
            flameEnd();
        }
        
        // Stay in this function for the full second to make sure we're not moving, before we allow the user to possibly retract
        delay( 500 - NO_ANGLE_SWING_DURATION );
    }
}

String startElectricHammerMove(int16_t speed) {
    hammer_in_motion = true;
    // Make sure we're vented
    safeDigitalWrite(VENT_VALVE_DO, LOW);
    // engage drive wheel
    safeDigitalWrite(RETRACT_VALVE_DO, HIGH);
    // wait for engagement
    delay(50);
    String movecmd("@05!G ");
    movecmd += speed;
    TurretRotationMotorSerial.println(movecmd);
    return movecmd;
}

void stopElectricHammerMove(void) {
    // disengage even if weapons are disabled
    digitalWrite(RETRACT_VALVE_DO, LOW);
    TurretRotationMotorSerial.println("@05!G 0");
    // wait for disengage
    delay(50);
    hammer_in_motion = false;
}

// use retract motor to gently move hammer
static void electricHammerMove(RCBitfield control, int16_t speed){
    String movecmd = startElectricHammerMove(speed);
    uint32_t inter_sbus_time = micros();
    while ((micros() - inter_sbus_time) < 30000UL) {
        bool working = isRadioConnected();
        if (working) {
            wdt_reset();
            uint16_t current_rc_bitfield = getRcBitfield();
            if (!(current_rc_bitfield & control)){
                break;
            }
            delay(5);
            TurretRotationMotorSerial.println(movecmd);
            inter_sbus_time = micros();
            // continue retracting
        } else {
            break;
        }
    }
    stopElectricHammerMove();
}

void gentleFire(RCBitfield control) {
    electricHammerMove(control, -1000);
}

void gentleRetract(RCBitfield control) {
    electricHammerMove(control,  1000);
}

void flameStart(){
    safeDigitalWrite(PROPANE_DO, HIGH);
}

void flameEnd(){
    // seems like this shouldn't require enable, even though disable should close valve itself
    digitalWrite(PROPANE_DO, LOW);
}

void valveSafe(){
    // Safing code deliberately does not use safeDigitalWrite since it should always go through.
    digitalWrite(ENABLE_VALVE_DO, LOW);
    digitalWrite(THROW_VALVE_DO, LOW);
    digitalWrite(VENT_VALVE_DO, LOW);
    digitalWrite(RETRACT_VALVE_DO, LOW);
    pinMode(ENABLE_VALVE_DO, OUTPUT);
    pinMode(THROW_VALVE_DO, OUTPUT);
    pinMode(VENT_VALVE_DO, OUTPUT);
    pinMode(RETRACT_VALVE_DO, OUTPUT);
}

void valveEnable(){
    // Assumes safe() has already been called beforehand, to set pin modes.
    safeDigitalWrite(ENABLE_VALVE_DO, HIGH);
}

void flameSafe(){
    digitalWrite(IGNITER_DO, LOW);
    digitalWrite(PROPANE_DO, LOW);
    pinMode(IGNITER_DO, OUTPUT);
    pinMode(PROPANE_DO, OUTPUT);
}

void flameEnable(){
    // Assumes safe() has already been called beforehand, to set pin modes.
    safeDigitalWrite(IGNITER_DO, HIGH);
}

void safeState(){
    valveSafe();
    flameSafe();
}

void enableState(){
   valveEnable();
}


