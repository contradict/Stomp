#include "turret_main.h"

#include "turretController.h"
#include "radioController.h"
#include "telemetryController.h"

//  The global TurretController Instance
//  
//  This instance owns all the other class instances it needs and is the main
//  dispatch for initilization, update, etc for those object instances
//
//  There is an extern decleration in turretController.h so everyone can have
//  the definition they need to access this golobl TurretController instance.


TurretController Turret;

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
    //  Initilize the three global objects

    Radio.Init();
    Turret.Init();
    Telem.Init();

    //  Restore persistant memory parameters

    Turret.RestoreParams();
    Telem.RestoreParams();
}

void loop()
{
    //  Update the three global objects

    Radio.Update();
    Turret.Update();
    Telem.Update();
}
