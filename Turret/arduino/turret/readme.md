
Files that were just removed because they were no longer necessary

* autodrive.cpp
* drive.cpp
* hold_down.cpp
* rc_pwm.cpp
* selfright.cpp
* telem_message_stream.h

Files that had there contents become a contoller object

New File                                Old File
------------------------------          ------------------------------
turretController.cpp                    chomp_main.cpp
radioController.cpp                     sbus.cpp
telemetryController.cpp                 telem.cpp & xbee.cpp
imuController.cpp                       imu.cpp
leddarController.cpp                    leddar_io.cpp
target.cpp                              object.cpp
targetTrackingController.cpp            track.cpp
targetAcquisitionController.cpp         targeting.cpp
autoFireController.cpp                  autofire.cpp

Controller Hierarchy

Global Objects                Sub Objects (instantiated in global object init())
--------------                --------------------------------------------------

TurretController -------------+--- TurretRotationController --------+--- AutoAimController
                              |
                              +--- HammerController
                              |
                              +--- FlameThrowerController
                              |
                              +--- IMUController
                              |
                              +--- AutoFireController

TargetTrackingController -----+--- TargetAcquisitionController ------+--- LeddarController

RadioController

TelemetryController