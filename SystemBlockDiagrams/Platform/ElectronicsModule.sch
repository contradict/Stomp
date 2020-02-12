EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 14 16
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 4800 2450 0    50   ~ 0
TODO:\n    IMU Connectors\n    Telem Radio Connectors\n    Ctrl radio connectors\n    Pressure Sensor\n    Pressure Sensor connectors\n
$Sheet
S 4000 4000 1450 2000
U 5E35FEB7
F0 "sheet5E35FE9E" 50
F1 "PowerDistribution.sch" 50
F2 "MCU_5V" I R 5450 4150 50 
F3 "MCU_GND" I R 5450 4250 50 
F4 "Ctrl_5V" I R 5450 4400 50 
F5 "Ctrl_GND" I R 5450 4500 50 
F6 "Telem_5V" I R 5450 4650 50 
F7 "Telem_GND" I R 5450 4750 50 
F8 "Pressure_5V" I R 5450 4900 50 
F9 "Pressure_GND" I R 5450 5000 50 
F10 "Leg0_24V" I L 4000 5200 50 
F11 "Leg0_GND" I L 4000 5300 50 
F12 "Leg1_24V" I L 4000 5450 50 
F13 "Leg1_GND" I L 4000 5550 50 
F14 "Leg2_24V" I L 4000 5700 50 
F15 "Leg2_GND" I L 4000 5800 50 
F16 "Leg3_GND" I R 5450 5800 50 
F17 "Leg3_24V" I R 5450 5700 50 
F18 "Leg4_GND" I R 5450 5550 50 
F19 "Leg4_24V" I R 5450 5450 50 
F20 "Leg5_GND" I R 5450 5300 50 
F21 "Leg5_24V" I R 5450 5200 50 
F22 "CAN_A" I L 4000 4150 50 
F23 "CAN_B" I L 4000 4250 50 
$EndSheet
Text Notes 4400 3400 0    50   ~ 0
Beagleboard Interface Cards\n    mikroBus Cape https://www.mikroe.com/beaglebone-mikrobus-cape\n      Legs - RS485 Click https://www.mikroe.com/rs485-5v-click\n      Pressure Sensors - ADC Click https://www.mikroe.com/adc-8-click\n      Battery Manager and S.BUS - CAN Click https://www.mikroe.com/can-spi-5v-click\n      Telemetry Radio and IMU - Adapter Click https://www.mikroe.com/adapter-click
$Comp
L Stomp:Tyranis_S.BUS R?
U 1 1 5E3614F3
P 9550 1400
AR Path="/5E3614F3" Ref="R?"  Part="1" 
AR Path="/5E35EB39/5E3614F3" Ref="R?"  Part="1" 
F 0 "R?" V 9763 1072 50  0000 R CNN
F 1 "Tyranis_S.BUS" V 9672 1072 50  0000 R CNN
F 2 "" H 9550 1400 50  0001 C CNN
F 3 "" H 9550 1400 50  0001 C CNN
	1    9550 1400
	0    -1   -1   0   
$EndComp
$Comp
L Stomp:RFD900+ R?
U 1 1 5E3614F9
P 9550 2650
AR Path="/5E3614F9" Ref="R?"  Part="1" 
AR Path="/5E35EB39/5E3614F9" Ref="R?"  Part="1" 
F 0 "R?" H 9575 3615 50  0000 C CNN
F 1 "RFD900+" H 9575 3524 50  0000 C CNN
F 2 "" H 9150 3700 50  0001 C CNN
F 3 "" H 9150 3700 50  0001 C CNN
	1    9550 2650
	1    0    0    -1  
$EndComp
$Comp
L Stomp:YostEmbeddedLX IMU?
U 1 1 5E361501
P 8350 1550
AR Path="/5E361501" Ref="IMU?"  Part="1" 
AR Path="/5E35EB39/5E361501" Ref="IMU?"  Part="1" 
F 0 "IMU?" H 8350 2025 50  0000 C CNN
F 1 "YostEmbeddedLX" H 8350 1934 50  0000 C CNN
F 2 "" H 8200 1900 50  0001 C CNN
F 3 "https://yostlabs.com/wp/wp-content/uploads/pdf/3-Space-Sensor-Users-Manual-Embedded-LX.pdf" H 8200 1900 50  0001 C CNN
F 4 "Yost Labs" H 8350 1550 50  0001 C CNN "Supplier"
F 5 "TSS_LX" H 8350 1550 50  0001 C CNN "Supplier Part Number"
	1    8350 1550
	1    0    0    -1  
$EndComp
$EndSCHEMATC
