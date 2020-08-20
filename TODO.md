# BeagleBone

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
  * see hull_control branch for current work.
  * https://github.com/csiro-robotics/syropod_highlevel_controller

## Telemetry
  * Radio works, `/dev/tty.mikrobus1`, 115200
  * Radio is RFD900x
  * Jo write a radio driver, works with cosmos. Need to start sending it data.
  * COSMOS upgrade

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

## Shutdown
  * BeagleBone does not power off after executing `halt -p`

# Leg Board

## Crash during startup with new leg_thread
  * Enfield communication dies
  * Just setting gain on one leg does not cause the problem

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
