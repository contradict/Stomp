#include "turret_main.h"
#include "DMASerial.h"

HardwareSerial& TurretRotationMotorSerial = Serial1;   // RX pin 19, TX pin 18
HardwareSerial& LeddarSerial = Serial2;  // RX pin 17, TX pin 16

volatile bool g_enabled = false;

void setup(){
  turretSetup();
}

void loop(){
  turretLoop();
}
