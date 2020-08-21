#include "turret_main.h"

#include "turretController.h"
#include "radioController.h"
#include "telemetryController.h"
#include "targetTrackingController.h"

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

// 
//  Redirect the setup and loop to the TurretController
//

void setup()
{
    //  Initilize the four global objects
    //  Order is important

    Telem.Init();
    Radio.Init();
    TargetTracking.Init();
    Turret.Init();

    //  Restore persistant memory parameters

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
