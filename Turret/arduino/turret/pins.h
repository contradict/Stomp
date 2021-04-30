#ifndef PINS_H
#define PINS_H

// ----------------- ANALOG PINS ----------------- 

// Sensors

#define HAMMER_ANGLE_AI A1
#define HAMMER_THROW_PRESSURE_AI A3
#define HAMMER_RETRACT_PRESSURE_AI A5

// ----------------- DIGITAL PINS ---------------

//  Flame Thrower Control

#define PROPANE_LEFT_DO 2
#define PROPANE_RIGTH_DO 3
#define IGNITERS_DO 5

//  Hammer Control

//  VERY IMPORTANT: In HammerController.cpp there is optimized code that requires
//  these pin asignments DONT change.  Chaning just here will break things.  

//#define THROW_PRESSURE_VALVE_DO 6
//#define THROW_VENT_VALVE_DO 7
//#define RETRACT_PRESSURE_VALVE_DO 8
//#define RETRACT_VENT_VALVE_DO 9

//  Hammer Pressure
#define THROW_PRESSURE_VALVE_DO 6
#define RETRACT_PRESSURE_VALVE_DO 7

//  Hammer Vent
#define THROW_VENT_VALVE_DO 8
#define RETRACT_VENT_VALVE_DO 9

//  Hammer Enable
#define HAMMER_ENABLE_DO A8


//  XBee

#define XBEE_CTS_DI A12


// ----------------- SERIAL BUS (4 USRT & 1 I2C) ---------------

//  Just here so there is one place to know what pins are in use

//  UART 0
//  s_TelemetrySerial - Shared (one or the other) on-board USB and XBee raido
//  TX0 - DIGITAL PIN 1
//  RX0 - DIGITAL PIN 0

//  UART 1
//  s_TurretRotationMotorSerial - Roboteq Motor Contoller
//  TX1 - DIGITAL PIN 18
//  RX1 - DIGITAL PIN 19

//  UART 2
//  s_LeddarSerial - Leddar Depth / Distance Sensor
//  TX2 - DIGITAL PIN 16
//  RX2 - DIGITAL PIN 17

//  UART 3
//  FrSKY Remote Control Raido
//  TX3 - DIGITAL PIN 14
//  RX4 - DIGITAL PIN 15

//  I2C
//  s_IMUSerial - MPU6050 Inertial measurement unit (IMU)
//  SDA - DIGITAL PIN 20
//  SCL - DIGITAL PINT 21

// ----------------- GLOBALS ----------------

extern volatile bool g_enabled;

#endif
