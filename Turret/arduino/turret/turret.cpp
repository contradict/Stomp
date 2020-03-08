//
//  The global TurretController Instance
//  
//  This instance owns all the other class instances it needs and is the main
//  dispatch for initilization, update, etc for those object instances
//
//  There is an extern decleration in turretController.h so everyone can have
//  the definition they need to access this golobl TurretController instance.
//
//  turret_main.cpp is still the main loop, but it passes control to 'Turret'
//  for it to do it's part.
//

#include "Arduino.h"
#include "turretController.h"

TurretController Turret;