EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
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
L Stomp:PneumaticServoValve SV?
U 1 1 5E1BF796
P 3250 1500
F 0 "SV?" H 3217 2165 50  0000 C CNN
F 1 "Enfield S2" H 3217 2074 50  0000 C CNN
F 2 "" H 2450 1200 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 2450 1200 50  0001 C CNN
	1    3250 1500
	-1   0    0    -1  
$EndComp
Text Notes 3750 850  0    50   ~ 0
Replace exiting connector with\nHirose HR10A-7R-6P(73) \nDigiKey HR1596-ND
Text Notes 5100 850  0    50   ~ 0
Mating connector is\nHirose  HR10A-7P-6S(73) \nDigiKey HR1588-ND
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E1C17F9
P 3250 2850
F 0 "SV?" H 3217 3515 50  0000 C CNN
F 1 "Enfield S2" H 3217 3424 50  0000 C CNN
F 2 "" H 2450 2550 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 2450 2550 50  0001 C CNN
	1    3250 2850
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E1C1C1A
P 3250 4250
F 0 "SV?" H 3217 4915 50  0000 C CNN
F 1 "Enfield S2" H 3217 4824 50  0000 C CNN
F 2 "" H 2450 3950 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 2450 3950 50  0001 C CNN
	1    3250 4250
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:AngleSensor U?
U 1 1 5E1C2B3D
P 850 1450
F 0 "U?" H 817 1955 50  0000 C CNN
F 1 "AngleSensor" H 817 1864 50  0000 C CNN
F 2 "" H 400 1200 50  0001 C CNN
F 3 "" H 400 1200 50  0001 C CNN
F 4 "TT Electronics" H 850 1450 50  0001 C CNN "Manufacturer"
F 5 "6127V1A60L.5" H 817 1773 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 850 1450 50  0001 C CNN "Supplier"
F 7 "858-6127V1A60L.5" H 850 1450 50  0001 C CNN "Supplier Part Number"
	1    850  1450
	-1   0    0    -1  
$EndComp
$Comp
L Connector:DIN-6 J?
U 1 1 5E1C5E68
P 4250 1600
F 0 "J?" H 4250 2171 50  0000 C CNN
F 1 "Hirose 6-Pin Male" H 4250 2080 50  0000 C CNN
F 2 "" H 4250 1600 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/hirose-electric-co-ltd/HR10A-7R-6P-73/HR1596-ND/1095454" H 4250 1600 50  0001 C CNN
F 4 "Hirose" H 4250 1600 50  0001 C CNN "Manufaturer"
F 5 "HR10A-7R-6P(73)" H 4250 1600 50  0001 C CNN "Manufacturer Part Number"
F 6 "DigiKey" H 4250 1600 50  0001 C CNN "Supplier"
F 7 "HR1596-ND" H 4250 1989 50  0000 C CNN "Supplier Part Number"
	1    4250 1600
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-6 J?
U 1 1 5E1C7296
P 5350 1600
F 0 "J?" H 5350 2171 50  0000 C CNN
F 1 "Hirose 6-Pin Female" H 5350 2080 50  0000 C CNN
F 2 "" H 5350 1600 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/hirose-electric-co-ltd/HR10A-7P-6S-73/HR1588-ND/1095446" H 5350 1600 50  0001 C CNN
F 4 "Hirose" H 5350 1600 50  0001 C CNN "Manufaturer"
F 5 "HR10A-7R-6S(73)" H 5350 1600 50  0001 C CNN "Manufacturer Part Number"
F 6 "DigiKey" H 5350 1600 50  0001 C CNN "Supplier"
F 7 "HR1588-ND" H 5350 1989 50  0000 C CNN "Supplier Part Number"
	1    5350 1600
	1    0    0    -1  
$EndComp
Text Notes 6000 1600 0    50   ~ 0
Cable\nMcMaster 75985K52
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E1CC057
P 9150 1550
F 0 "J?" H 9200 1957 50  0000 C CNN
F 1 "Micro-Fit 3.0 Socket Vertical" H 9200 1866 50  0000 C CNN
F 2 "" H 9150 1550 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0447690601/23-0447690601-ND/513217" H 9150 1550 50  0001 C CNN
F 4 "Digi-Key" H 9150 1550 50  0001 C CNN "Supplier"
F 5 " 23-0447690601-ND " H 9200 1775 50  0000 C CNN "Supplier Part Number"
	1    9150 1550
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E1CC73D
P 10400 1550
F 0 "J?" H 10450 1957 50  0000 C CNN
F 1 "Micro-Fit 3.0 Socket Rt Angle" H 10450 1866 50  0000 C CNN
F 2 "" H 10400 1550 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0447690601/23-0447690601-ND/513217" H 10400 1550 50  0001 C CNN
F 4 "Digi-Key" H 10400 1550 50  0001 C CNN "Supplier"
F 5 " WM1982-ND " H 10450 1775 50  0000 C CNN "Supplier Part Number"
	1    10400 1550
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E1CE321
P 7200 1550
F 0 "J?" H 7250 1957 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7250 1866 50  0000 C CNN
F 2 "" H 7200 1550 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430200601/WM2762-ND/1132438" H 7200 1550 50  0001 C CNN
F 4 "DigiKey" H 7200 1550 50  0001 C CNN "Supplier"
F 5 " WM2762-ND " H 7250 1775 50  0000 C CNN "Supplier Part Number"
	1    7200 1550
	1    0    0    -1  
$EndComp
Text Notes 6900 1950 0    50   ~ 0
Pins 20-24AWG, Gold\nMolex 0430310002\nDigiKey  WM1127CT-ND 
$Comp
L Connector:Conn_01x03_Female J?
U 1 1 5E1CF300
P 10400 2150
F 0 "J?" H 10428 2176 50  0000 L CNN
F 1 "Micro-Fit 3.0 RT Angle" H 10050 2400 50  0000 L CNN
F 2 "" H 10400 2150 50  0001 C CNN
F 3 "~" H 10400 2150 50  0001 C CNN
F 4 "Mouser" H 10400 2150 50  0001 C CNN "Supplier"
F 5 " 538-43650-0304 " H 10400 2150 50  0001 C CNN "Supplier Part Number"
F 6 "Molex" H 10400 2150 50  0001 C CNN "Manufacturer"
F 7 "43650-0304" H 10400 2150 50  0001 C CNN "Manufacturer part Number"
	1    10400 2150
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Female J?
U 1 1 5E1CFBFF
P 9300 2150
F 0 "J?" H 9328 2176 50  0000 L CNN
F 1 "Micro-Fit 3.0 Vertical" H 8950 2400 50  0000 L CNN
F 2 "" H 9300 2150 50  0001 C CNN
F 3 "~" H 9300 2150 50  0001 C CNN
F 4 "Mouser" H 9300 2150 50  0001 C CNN "Supplier"
F 5 "538-43650-0319" H 9300 2150 50  0001 C CNN "Supplier Part Number"
F 6 "Molex" H 9300 2150 50  0001 C CNN "Manufacturer"
F 7 "43650-0319" H 9300 2150 50  0001 C CNN "Manufacturer part Number"
	1    9300 2150
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E1D13D7
P 1350 1450
F 0 "J?" H 1458 1731 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 1458 1640 50  0000 C CNN
F 2 "" H 1350 1450 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 1350 1450 50  0001 C CNN
F 4 "Molex" H 1350 1450 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 1350 1450 50  0001 C CNN "Manufacturer Part Number"
F 6 "DigiKey" H 1350 1450 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 1350 1450 50  0001 C CNN "Supplier Part Number"
	1    1350 1450
	1    0    0    -1  
$EndComp
Text Notes 1150 1850 0    50   ~ 0
Pins 20-24AWG, Gold\nMolex 0430310002\nDigiKey  WM1127CT-ND 
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J?
U 1 1 5E1D7468
P 10350 2700
F 0 "J?" H 10400 3007 50  0000 C CNN
F 1 "Micro-Fit 3.0 RT Angle" H 10400 2916 50  0000 C CNN
F 2 "" H 10350 2700 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0447640401/23-0447640401-ND/513244" H 10350 2700 50  0001 C CNN
F 4 "DigiKey" H 10350 2700 50  0001 C CNN "Supplier"
F 5 "23-0447640401-ND" H 10400 2825 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 10350 2700 50  0001 C CNN "Manufacturer"
F 7 "0447640401" H 10350 2700 50  0001 C CNN "Manufacturer Part Number"
	1    10350 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J?
U 1 1 5E1D8086
P 9150 2700
F 0 "J?" H 9200 3007 50  0000 C CNN
F 1 "Micro-Fit 3.0 Vertical" H 9200 2916 50  0000 C CNN
F 2 "" H 9150 2700 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0447690401/23-0447690401-ND/513216" H 9150 2700 50  0001 C CNN
F 4 "DigiKey" H 9150 2700 50  0001 C CNN "Supplier"
F 5 "23-0447690401-ND" H 9200 2825 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 9150 2700 50  0001 C CNN "Manufacturer"
F 7 " 0447690401" H 9150 2700 50  0001 C CNN "Manufacturer Part Number"
	1    9150 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J?
U 1 1 5E1D94F3
P 7200 2650
F 0 "J?" H 7250 2957 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7250 2866 50  0000 C CNN
F 2 "" H 7200 2650 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250400/WM1784-ND/252497" H 7200 2650 50  0001 C CNN
F 4 "DigiKey" H 7200 2650 50  0001 C CNN "Supplier"
F 5 "WM1784-ND" H 7250 2775 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 7200 2650 50  0001 C CNN "Manufacturer"
F 7 "0430250400" H 7200 2650 50  0001 C CNN "Manufacturer part Number"
	1    7200 2650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
