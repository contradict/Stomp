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
TODO:\n    Pressure Sensor\n    Pressure Sensor connectors\n
$Sheet
S 1200 5150 1450 2000
U 5E35FEB7
F0 "sheet5E35FE9E" 50
F1 "PowerDistribution.sch" 50
F2 "MCU_5V" I R 2650 5300 50 
F3 "MCU_GND" I R 2650 5400 50 
F4 "Ctrl_5V" I R 2650 5550 50 
F5 "Ctrl_GND" I R 2650 5650 50 
F6 "Telem_5V" I R 2650 5800 50 
F7 "Telem_GND" I R 2650 5900 50 
F8 "Pressure_5V" I R 2650 6050 50 
F9 "Pressure_GND" I R 2650 6150 50 
F10 "Leg0_24V" I L 1200 6350 50 
F11 "Leg0_GND" I L 1200 6450 50 
F12 "Leg1_24V" I L 1200 6600 50 
F13 "Leg1_GND" I L 1200 6700 50 
F14 "Leg2_24V" I L 1200 6850 50 
F15 "Leg2_GND" I L 1200 6950 50 
F16 "Leg3_GND" I R 2650 6950 50 
F17 "Leg3_24V" I R 2650 6850 50 
F18 "Leg4_GND" I R 2650 6700 50 
F19 "Leg4_24V" I R 2650 6600 50 
F20 "Leg5_GND" I R 2650 6450 50 
F21 "Leg5_24V" I R 2650 6350 50 
F22 "CAN_A" I L 1200 5300 50 
F23 "CAN_B" I L 1200 5400 50 
$EndSheet
Text Notes 4400 3400 0    50   ~ 0
Beagleboard Interface Cards\n    mikroBus Cape https://www.mikroe.com/beaglebone-mikrobus-cape\n        2 - Legs - RS485 Click https://www.mikroe.com/rs485-5v-click\n        1 - Pressure Sensors - ADC Click https://www.mikroe.com/adc-8-click\n        4 - Battery Manager and S.BUS\n            bottom - Terminal Click https://www.mikroe.com/terminal-click\n            top - CAN Click https://www.mikroe.com/can-spi-5v-click\n        3 - Telemetry Radio and IMU - Adapter Click https://www.mikroe.com/adapter-click
$Comp
L Stomp:Tyranis_S.BUS R?
U 1 1 5E3614F3
P 3350 6400
AR Path="/5E3614F3" Ref="R?"  Part="1" 
AR Path="/5E35EB39/5E3614F3" Ref="R1"  Part="1" 
F 0 "R1" V 3563 6072 50  0000 R CNN
F 1 "Tyranis_S.BUS" V 3472 6072 50  0000 R CNN
F 2 "" H 3350 6400 50  0001 C CNN
F 3 "" H 3350 6400 50  0001 C CNN
	1    3350 6400
	0    -1   -1   0   
$EndComp
$Comp
L Stomp:RFD900+ R?
U 1 1 5E3614F9
P 7100 1250
AR Path="/5E3614F9" Ref="R?"  Part="1" 
AR Path="/5E35EB39/5E3614F9" Ref="R2"  Part="1" 
F 0 "R2" H 7125 2215 50  0000 C CNN
F 1 "RFD900+" H 7125 2124 50  0000 C CNN
F 2 "" H 6700 2300 50  0001 C CNN
F 3 "" H 6700 2300 50  0001 C CNN
	1    7100 1250
	0    1    1    0   
$EndComp
$Comp
L Stomp:YostEmbeddedLX IMU?
U 1 1 5E361501
P 10000 1100
AR Path="/5E361501" Ref="IMU?"  Part="1" 
AR Path="/5E35EB39/5E361501" Ref="IMU1"  Part="1" 
F 0 "IMU1" H 10000 1575 50  0000 C CNN
F 1 "YostEmbeddedLX" H 10000 1484 50  0000 C CNN
F 2 "" H 9850 1450 50  0001 C CNN
F 3 "https://yostlabs.com/wp/wp-content/uploads/pdf/3-Space-Sensor-Users-Manual-Embedded-LX.pdf" H 9850 1450 50  0001 C CNN
F 4 "Yost Labs" H 10000 1100 50  0001 C CNN "Supplier"
F 5 "TSS_LX" H 10000 1100 50  0001 C CNN "Supplier Part Number"
	1    10000 1100
	1    0    0    -1  
$EndComp
$Comp
L Stomp:ADC_Click CK4
U 1 1 5E493C9A
P 9800 4100
F 0 "CK4" H 9400 4800 50  0000 L CNN
F 1 "ADC_Click" H 10000 4150 50  0000 L CNN
F 2 "" H 9350 3300 50  0001 C CNN
F 3 "https://www.mikroe.com/adc-click" H 9350 3300 50  0001 C CNN
F 4 "Mikroe" H 9800 4100 50  0001 C CNN "Manufacturer"
F 5 "Digi-key" H 9800 4100 50  0001 C CNN "Supplier"
F 6 "1471-1301-ND" H 9800 4100 50  0001 C CNN "Supplier Part Number"
F 7 "MIKROE-922" H 9800 4100 50  0001 C CNN "Manufacturer Part Number"
	1    9800 4100
	1    0    0    -1  
$EndComp
$Comp
L Stomp:CAN_Click CK2
U 1 1 5E494290
P 4750 7000
F 0 "CK2" H 4300 7300 50  0000 L CNN
F 1 "CAN_Click" H 4800 7000 50  0000 L CNN
F 2 "" H 4250 6850 50  0001 C CNN
F 3 "https://www.mikroe.com/can-spi-33v-click" H 4250 6850 50  0001 C CNN
F 4 "Mikroe" H 4750 7000 50  0001 C CNN "Manufacturer"
F 5 "Digi-Key" H 4750 7000 50  0001 C CNN "Supplier"
F 6 "1471-1324-ND" H 4750 7000 50  0001 C CNN "Supplier Part Number"
F 7 "MIKROE-986" H 4750 7000 50  0001 C CNN "Manufacturer Part Number"
	1    4750 7000
	1    0    0    -1  
$EndComp
$Comp
L Stomp:RS485_Click CK1
U 1 1 5E4970DF
P 1350 3100
F 0 "CK1" H 1750 2700 50  0000 C CNN
F 1 "RS485_Click" H 1550 3050 50  0000 C CNN
F 2 "" H 900 2800 50  0001 C CNN
F 3 "https://www.mikroe.com/rs485-33v-click" H 900 2800 50  0001 C CNN
F 4 "Mikroe" H 1350 3100 50  0001 C CNN "Manufacturer"
F 5 "MIKROE-989" H 1350 3100 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 1350 3100 50  0001 C CNN "Supplier"
F 7 "1471-1327-ND" H 1350 3100 50  0001 C CNN "Supplier Part Number"
	1    1350 3100
	-1   0    0    1   
$EndComp
$Comp
L Stomp:Terminal_Click CK3
U 1 1 5E49AB80
P 4850 5250
F 0 "CK3" H 4350 6450 50  0000 L CNN
F 1 "Terminal_Click" H 4900 5250 50  0000 L CNN
F 2 "" H 4850 5250 50  0001 C CNN
F 3 "https://www.mikroe.com/terminal-click" H 4850 5250 50  0001 C CNN
F 4 "Mikroe" H 4850 5250 50  0001 C CNN "Manufacturer"
F 5 "MIKROE-3745" H 4850 5250 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4850 5250 50  0001 C CNN "Supplier"
F 7 "1471-MIKROE-3745-ND" H 4850 5250 50  0001 C CNN "Supplier Part Number"
	1    4850 5250
	1    0    0    -1  
$EndComp
$Comp
L Stomp:Adapter_Click CK5
U 1 1 5E49C9C8
P 9900 2250
F 0 "CK5" H 10478 2321 50  0000 L CNN
F 1 "Adapter_Click" H 10478 2230 50  0000 L CNN
F 2 "" H 9300 1500 50  0001 C CNN
F 3 "https://www.mikroe.com/adapter-click" H 9300 1500 50  0001 C CNN
F 4 "Mikroe" H 9900 2250 50  0001 C CNN "Manufacturer"
F 5 "MIKROE-1432" H 9900 2250 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9900 2250 50  0001 C CNN "Supplier"
F 7 "1471-1107-ND" H 9900 2250 50  0001 C CNN "Supplier Part Number"
	1    9900 2250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x05_Odd_Even J132
U 1 1 5E49DB90
P 8700 2200
F 0 "J132" H 8750 2617 50  0000 C CNN
F 1 "Conn_10_M" H 8750 2526 50  0000 C CNN
F 2 "" H 8700 2200 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/901303210_sd.pdf" H 8700 2200 50  0001 C CNN
F 4 "Molex" H 8700 2200 50  0001 C CNN "Manufacturer"
F 5 "0901303210" H 8700 2200 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 8700 2200 50  0001 C CNN "Supplier"
F 7 "WM8277-ND" H 8700 2200 50  0001 C CNN "Supplier Part Number"
	1    8700 2200
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J128
U 1 1 5E49EACE
P 3700 5350
F 0 "J128" H 3750 5867 50  0000 C CNN
F 1 "Conn_16_M" H 3750 5776 50  0000 C CNN
F 2 "" H 3700 5350 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/901303216_sd.pdf" H 3700 5350 50  0001 C CNN
F 4 "Molex" H 3700 5350 50  0001 C CNN "Manufacturer"
F 5 "0901303216" H 3700 5350 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 3700 5350 50  0001 C CNN "Supplier"
F 7 "23-0901303216-ND" H 3700 5350 50  0001 C CNN "Supplier Part Number"
	1    3700 5350
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J129
U 1 1 5E49FE6A
P 4800 1250
F 0 "J129" H 4850 1767 50  0000 C CNN
F 1 "Conn_16_F" H 4850 1676 50  0000 C CNN
F 2 "" H 4800 1250 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/901420010_sd.pdf" H 4800 1250 50  0001 C CNN
F 4 "Molex" H 4800 1250 50  0001 C CNN "Manufacturer"
F 5 "0901420010" H 4800 1250 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4800 1250 50  0001 C CNN "Supplier"
F 7 "WM8037-ND" H 4800 1250 50  0001 C CNN "Supplier Part Number"
	1    4800 1250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J130
U 1 1 5E4AC3EF
P 5800 1250
F 0 "J130" H 5850 1767 50  0000 C CNN
F 1 "Conn_16_M" H 5850 1676 50  0000 C CNN
F 2 "" H 5800 1250 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/901303216_sd.pdf" H 5800 1250 50  0001 C CNN
F 4 "Molex" H 5800 1250 50  0001 C CNN "Manufacturer"
F 5 "0901303216" H 5800 1250 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 5800 1250 50  0001 C CNN "Supplier"
F 7 "23-0901303216-ND" H 5800 1250 50  0001 C CNN "Supplier Part Number"
	1    5800 1250
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J127
U 1 1 5E4ADD3D
P 3100 5350
F 0 "J127" H 3150 5867 50  0000 C CNN
F 1 "Conn_16_F" H 3150 5776 50  0000 C CNN
F 2 "" H 3100 5350 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/901420010_sd.pdf" H 3100 5350 50  0001 C CNN
F 4 "Molex" H 3100 5350 50  0001 C CNN "Manufacturer"
F 5 "0901420010" H 3100 5350 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 3100 5350 50  0001 C CNN "Supplier"
F 7 "WM8037-ND" H 3100 5350 50  0001 C CNN "Supplier Part Number"
	1    3100 5350
	1    0    0    -1  
$EndComp
$Comp
L Stomp:BEAGLEBONE_AI BBAI1
U 1 1 5E5170DD
P 6500 4550
F 0 "BBAI1" H 6878 4621 50  0000 L CNN
F 1 "BEAGLEBONE_AI" H 6878 4530 50  0000 L CNN
F 2 "" H 6500 4550 50  0001 C CNN
F 3 "https://beagleboard.org/ai" H 6500 4550 50  0001 C CNN
F 4 "Beagleboard.org" H 6500 4550 50  0001 C CNN "Manufacturer"
F 5 "BBONE-AI" H 6500 4550 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6500 4550 50  0001 C CNN "Supplier"
F 7 "BBONE-AI-ND" H 6500 4550 50  0001 C CNN "Supplier Part Number"
	1    6500 4550
	1    0    0    -1  
$EndComp
$Comp
L Stomp:Click_Cape CP1
U 1 1 5E5177AC
P 6500 5050
F 0 "CP1" H 6778 5096 50  0000 L CNN
F 1 "Click_Cape" H 6778 5005 50  0000 L CNN
F 2 "" H 6500 5050 50  0001 C CNN
F 3 "https://www.mikroe.com/beaglebone-mikrobus-cape" H 6500 5050 50  0001 C CNN
F 4 "Mikroe" H 6500 5050 50  0001 C CNN "Manufacturer"
F 5 "MIKROE-1857" H 6500 5050 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6500 5050 50  0001 C CNN "Supplier"
F 7 "1471-1429-ND" H 6500 5050 50  0001 C CNN "Supplier Part Number"
	1    6500 5050
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x05_Odd_Even J131
U 1 1 5E5184CE
P 8050 2200
F 0 "J131" H 8100 2617 50  0000 C CNN
F 1 "Conn_10_F" H 8100 2526 50  0000 C CNN
F 2 "" H 8050 2200 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/901420010_sd.pdf" H 8050 2200 50  0001 C CNN
F 4 "Molex" H 8050 2200 50  0001 C CNN "Manufacturer"
F 5 "0901420010" H 8050 2200 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 8050 2200 50  0001 C CNN "Supplier"
F 7 "WM8037-ND" H 8050 2200 50  0001 C CNN "Supplier Part Number"
	1    8050 2200
	1    0    0    -1  
$EndComp
Text Notes 1700 1250 0    50   ~ 0
C-Grid connector pins\n     22-24AWG  Digi-Key:WM2559-ND Molex:0901192110\n     26-28AWG Digi-Key:WM2561-ND Molex:0901192121
$EndSCHEMATC
