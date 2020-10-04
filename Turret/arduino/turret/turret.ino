#include "turret_main.h"
#include "pins.h"

#include "turretController.h"
#include "radioController.h"
#include "telemetryController.h"
#include "targetTrackingController.h"
#include "sensors.h"

//  The global TurretController Instance
//  
//  This instance owns all the other class instances it needs and is the main
//  dispatch for initilization, update, etc for those object instances
//
//  There is an extern decleration in turretController.h so everyone can have
//  the definition they need to access this golobl TurretController instance.

TurretController Turret;

//  The global TargetTrackingController Instance
//
//  There is an extern decleration in targetTrackingController.h so everyone can have
//  the definition they need to access this golobl RadioController instance.

TargetTrackingController TargetTracking;

//  The global RadioController Instance
//
//  There is an extern decleration in radioController.h so everyone can have
//  the definition they need to access this golobl RadioController instance.

RadioController Radio;

//  The global TelemetryController Instance
//
//  There is an extern decleration in telemetryController.h so everyone can have
//  the definition they need to access this golobl RadioController instance.

TelemetryController Telem;

volatile bool g_enabled = false;

//  FORCE ALL IMPORTANT OUTPUTS TO LOW FIRST THING WHEN WE BOOT

void safe()
{
    digitalWrite(HAMMER_ENABLE_DO, LOW);
    digitalWrite(THROW_PRESSURE_VALVE_DO, LOW);
    digitalWrite(THROW_VENT_VALVE_DO, LOW);
    digitalWrite(RETRACT_PRESSURE_VALVE_DO, LOW);
    digitalWrite(RETRACT_VENT_VALVE_DO, LOW);

    digitalWrite(IGNITERS_DO, LOW);
    digitalWrite(PROPANE_LEFT_DO, LOW);
    digitalWrite(PROPANE_RIGTH_DO, LOW);

    pinMode(HAMMER_ENABLE_DO, OUTPUT);
    pinMode(THROW_PRESSURE_VALVE_DO, OUTPUT);
    pinMode(THROW_VENT_VALVE_DO, OUTPUT);
    pinMode(RETRACT_PRESSURE_VALVE_DO, OUTPUT);
    pinMode(RETRACT_VENT_VALVE_DO, OUTPUT);

    pinMode(IGNITERS_DO, OUTPUT);
    pinMode(PROPANE_LEFT_DO, OUTPUT);
    pinMode(PROPANE_RIGTH_DO, OUTPUT);
}

// 
//  Redirect the setup and loop to the TurretController
//

void setup()
{
    safe();
    
    //  Initilize the four global objects
    //  Order is important

    Telem.Init();
    Radio.Init();
    TargetTracking.Init();
    Turret.Init();

    //  Restore persistant memory parameters
    restoreSensorParameters();
    Telem.RestoreParams();
    TargetTracking.RestoreParams();
    Turret.RestoreParams();
}

void loop()
{
    //  Update the four global objects
    //  Order is important

    Radio.Update();
    TargetTracking.Update();
    Turret.Update();
    Telem.Update();
}
