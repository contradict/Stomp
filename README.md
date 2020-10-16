# Stomp - Software for the Pneumatic Legged Robot "Chomp"

This repository contains the control software for Chomp the BattleBot.
Presently, Chomp has two entirely separate systems the Hull and the Turret. Each
is entirely self-contained with its own power and control systems. A rough
[schematic](SystemBlockDiagrams) of the Hull is available.

## Hull

The Hull consists has 6 leg modules, each has a control [board](LegBoard) for local joint
coordination. This board uses an STM32F722 microcontroller to communicate with
the joint angle sensors, the Enfield S2 servos, and the central microcontroller.
This processor measures the joint angles and provides pneumatic cylinder length
feedback as an analog voltage to the servos. Commands are provided over
an RS485 serial connection using the Modbus protocol and can configure the
servos, command a toe position, and provide status information such as measured
leg position and cylinder pressures.

The central microcontroller is a BeagleBone AI running Debian Linux. Two
executables are used during operation
[sbus_radio](Hull/HullControl/src/sbus_radio) reads serial data from a FrSky
receiver and emits a stream of data using [LCM](https://lcm-proj.github.io/).
[leg_control](Hull/HullControl/src/leg_control) listens to these messages and
passes them to a realtime
([SCHED_FIFO](https://man7.org/linux/man-pages/man7/sched.7.html)) thread. This
thread determines the desired toe positions and communicates with the legs over
the RS485 bus.

## Turret

The Turret uses the control electronics from the previous revision of
[Chomp](https://www.github.com/jascahlittle/Chomp). This uses a single Arduino
Mega with an ATMega328P to switch the pneumatic valves for firing the hammer and
communicate with the control radio, turret rotation motor controller, LEDDAR, and
telemetry radio. There is a nice overview of the control software
[here](Turret/arduino/turret). This software handles the automatic tracking and
hammer firing using the ranging data from the LEDDAR, as well as precisely
scheduling the valve timings to ensure a clean throw and retract of the hammer.
