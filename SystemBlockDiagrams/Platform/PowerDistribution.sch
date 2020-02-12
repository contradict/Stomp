EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 15 16
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Stomp:CUIPYBE PS?
U 1 1 5E26E04C
P 5100 1700
F 0 "PS?" H 5100 2025 50  0000 C CNN
F 1 "CUIPYBE" H 5100 1934 50  0000 C CNN
F 2 "" H 4900 1900 50  0001 C CNN
F 3 "" H 4900 1900 50  0001 C CNN
	1    5100 1700
	1    0    0    -1  
$EndComp
$Sheet
S 5100 2600 1000 1650
U 5E26E1F9
F0 "Battery Sled" 50
F1 "BaterySled.sch" 50
F2 "CAN_A" I L 5100 2850 50 
F3 "CAN_B" I L 5100 3000 50 
F4 "+24V" I R 6100 3200 50 
F5 "GND" I R 6100 3350 50 
$EndSheet
Text Notes 7050 1750 0    50   ~ 0
TODO:\n    LED Strip\n    Fuses\n    Disconnect Switch\n    Power Connectors
$Comp
L Device:LED_Series D?
U 1 1 5E280DCF
P 7600 2400
F 0 "D?" H 7600 2680 50  0000 C CNN
F 1 "LED_Series" H 7600 2589 50  0000 C CNN
F 2 "" H 7500 2400 50  0001 C CNN
F 3 "~" H 7500 2400 50  0001 C CNN
	1    7600 2400
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_DPST SW?
U 1 1 5E2817A6
P 7050 3250
F 0 "SW?" H 7050 3575 50  0000 C CNN
F 1 "CustomDisconnect" H 7050 3484 50  0000 C CNN
F 2 "" H 7050 3250 50  0001 C CNN
F 3 "~" H 7050 3250 50  0001 C CNN
F 4 "" H 7050 3250 50  0001 C CNN "Field4"
	1    7050 3250
	1    0    0    -1  
$EndComp
$EndSCHEMATC
