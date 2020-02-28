// functions to send serial drive commands to Roboteq motor controllers 
#include "Arduino.h"
#include "drive.h"
#include "pins.h"
#include "telem.h"

// Serial out pins defined in turret.ino-- check there to verify proper connectivity to motor controller
extern HardwareSerial& DriveSerial;

void driveInit() {
    DriveSerial.begin(115200);
    DriveSerial.println("@00^CPRI 1 0");  // set serial priority first
    delay(5);
    DriveSerial.println("@00^CPRI 2 1");  // set RC priority second
    delay(5);
    DriveSerial.println("@00^ECHOF 1");  // turn off command echo
    delay(5);
    DriveSerial.println("@00^RWD 100");  // set RS232 watchdog to 100 ms
}

void drive( int16_t &p_speed) {
    p_speed = constrain(p_speed, -1000, 1000);

    // send "@nn!G mm" over software serial. mm is a command value, -1000 to 1000. nn is node number in RoboCAN network.
    
    DriveSerial.print("@01!G ");
    DriveSerial.println(p_speed);
    DriveSerial.print("@02!G ");
    DriveSerial.println(p_speed);
    DriveSerial.print("@03!G ");
    DriveSerial.println(p_speed);
    DriveSerial.print("@04!G ");
    DriveSerial.println(p_speed);
}

#define VOLTAGE_RESPONSE_LENGTH 12
#define VOLTAGE_RESPONSE_TIMEOUT 5000
#define NUM_DRIVES 5
void driveTelem(void) {
    //  BB MJS: Don't send anything right now
    /*
    static int idx = 0;
    static int16_t volts[NUM_DRIVES];
    char volt_buffer[VOLTAGE_RESPONSE_LENGTH];
    if(isTLMEnabled(TLM_ID_DRV)) {
        while(DriveSerial.available()) DriveSerial.read();
        String request("@0");
        request += (idx+1);
        request += "?V 2_";
        DriveSerial.write(request.c_str());
        uint32_t now = micros();
        while(DriveSerial.available()<VOLTAGE_RESPONSE_LENGTH &&
              micros() - now < VOLTAGE_RESPONSE_TIMEOUT);
        if(DriveSerial.available() >= VOLTAGE_RESPONSE_LENGTH) {
            DriveSerial.readBytes(volt_buffer, VOLTAGE_RESPONSE_LENGTH);
            // response is "@0i V=nnn\x0d+\x0d"
            // i=[1-NUM_DRIVES], nnn=volts*10
            volt_buffer[9] = '\x00';
            volts[idx++] = atoi(volt_buffer+6);
        } else {
            volts[idx++] = -1;
        }
        if(idx == NUM_DRIVES) {
            // id 1-4 are wheels, id 5 is weapons
            sendDriveTelem(reinterpret_cast<int16_t(&)[4]>(volts), volts[4]);
            idx = 0;
        }
    }
    */
}
