//
//  The global TurretController Instance
//  
//  This instance has all the other class instances it needs and is the main
//  dispatch for initilization and update for all other controllers
//
//  All non-TurretController controllers keep their own data or ask
//  this TurretController instance for access to data
//

#include "Arduino.h"
#include "turretController.h"

TurretController Turret;