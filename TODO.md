# BeagleBone

## System startup

  * Start telemetry on system boot

## Servo tuning while walking

  * leg_control listens on LEG_COMMAND for packets to send on modbus
  * Need to write tool to pack and send
  * Simultaneous on all legs
  * Enfield gain values (Gain, Damping, derivative, etc)

## Leg tool **#NextYear**

The feature set for a magical do-everything tool to configure the robot
firmware. For now, see Hull/Test/modbus_ui.c

  * Select leg by address
    * Scan for choices
  * Firmware update over modbus (as soon as the firmware supports it)!!!
  * Change address of selected leg
  * Display all feedback
    * Set polling frequency
    * Sensor Voltage
    * Angle
    * Length
    * Output voltage
    * Enfield command input
    * Rod pressure
    * Base pressure
    * adjust linearization parameters
      * min, max sensor voltage
      * min, max joint angle
      * min, max cylinder length
      * min, max output voltage
  * Tune servos
    * Set feedback polling frequency
    * All servo parameters (14 total)
    * All servo toggles (5 total)
    * Servo serial
    * Servo firmware rev
    * Plot command, feedback position
    * Manual position command
    * signal generator
      * set update period
      * sin/square
      * set signal period
      * set signal amplitude
      * set signal zero level
  * Command toe position

## Gait Generation
  * https://github.com/csiro-robotics/syropod_highlevel_controller
  * Optimization based gait
    * Find edges of leg working space? - Toe position -> joint angles -> joint
      angle limits.
    * At least three legs on the ground, not all on the same side of the robot
    * maintain specified velocity
  * Crabbing

## Telemetry
  * Radio works, `/dev/tty.mikrobus1`, 115200
  * Radio is RFD900x
  * Get telemetry branch merged!

## Pressure ADC
  * Uses a Mikro ADC-Click
  * Pins and driver enabled by [overlay](https://github.com/contradict/BeagleBoard-DeviceTrees/blob/v4.19.x-ti/src/arm/BBAI-SPI3-ADC-IMU.dts)
  * Pins show correct data on oscilloscope, zeros always arrive in software.
  * Wrong config on MISO?
  * Not actually the correct driver?
  * Reading the wrong file?

## IMU
  * Pins enabled by [overlay](https://github.com/contradict/BeagleBoard-DeviceTrees/blob/v4.19.x-ti/src/arm/BBAI-SPI3-ADC-IMU.dts)
  * Untested

## WiFi
  * hostapd configured /etc/hostapd.conf, SSID ChompTheBattlebot
  * interface configured /etc/network/interfaces wlan0 192.168.4.1/28
  * dnsmasq configured /etc/dnsmasq.conf DHCP range 192.168.4.2-14
  * DHCP requests arrive, responses sent (tcpdump)
  * Client never receives DHCP response (tcpdump)

# Leg Board

## Optimize kinematics
  * Complex results usually just use real part, don't compute complex part
  * Would DH parameters be better?

## Firmware update over modbus **#NextYear**

  * Firmare uses sectors 5, 6, 7, IVT in Sector 2
  * Bootloader in sector 4, IVT in Sector 1
  * Storage in Sector 3
  * On reset, bootloader check memory for special value, if not found sets
    VTOR for sector 2 and jumps to firmware reset

## Auto-addressing **#NextYear**
  * All boards boot with only all-call address (0x255)
  * Any board who's `UPSTREAM` input is high also responds to address 0x55
  * Modbus holding register write to change address (0x01)
  * Modbus coil write to set `DOWNSTREAM` output (0x01)
  * On boot, all `DOWNSTREAM` outputs are off.
  * To start addressing
      1. BeagleBone sets first `UPSTREAM`. GPIO needs to be wired on RS485
         Click.
      2. BeagleBone sets 0x55 board's address with write to 0x01
      3. BeagleBone de-asserts `UPSTREAM`
      4. BeagleBone asks newly addressed device to raise `DOWNSTREAM`
      5. BeagleBone sets 0x55 board's address with write to 0x01
      6. BeagleBone asks currently asserted device to de-assert
      7. repeat from 4 until all boards addressed
