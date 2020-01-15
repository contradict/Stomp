EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 2
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
P 1500 1350
F 0 "SV?" H 1467 2015 50  0000 C CNN
F 1 "Enfield S2" H 1467 1924 50  0000 C CNN
F 2 "" H 700 1050 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 700 1050 50  0001 C CNN
	1    1500 1350
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E1CE321
P 7100 1450
F 0 "J?" H 7150 1857 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7150 1766 50  0000 C CNN
F 2 "" H 7100 1450 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430200601/WM2762-ND/1132438" H 7100 1450 50  0001 C CNN
F 4 "Digi-Key" H 7100 1450 50  0001 C CNN "Supplier"
F 5 " WM2762-ND " H 7150 1675 50  0000 C CNN "Supplier Part Number"
	1    7100 1450
	1    0    0    -1  
$EndComp
Text Notes 6200 800  0    50   ~ 0
Microfit Male Pins 20-24AWG, Gold\nMolex 0430310002\nDigi-Key  WM1127CT-ND 
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E1D13D7
P 7050 2050
F 0 "J?" H 7158 2331 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7158 2240 50  0000 C CNN
F 2 "" H 7050 2050 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 7050 2050 50  0001 C CNN
F 4 "Molex" H 7050 2050 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 7050 2050 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 7050 2050 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 7150 1850 50  0000 C CNN "Supplier Part Number"
	1    7050 2050
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J?
U 1 1 5E1D94F3
P 10650 1000
F 0 "J?" H 10700 1307 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 10700 1216 50  0000 C CNN
F 2 "" H 10650 1000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250400/WM1784-ND/252497" H 10650 1000 50  0001 C CNN
F 4 "DigiKey" H 10650 1000 50  0001 C CNN "Supplier"
F 5 "WM1784-ND" H 10700 1125 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 10650 1000 50  0001 C CNN "Manufacturer"
F 7 "0430250400" H 10650 1000 50  0001 C CNN "Manufacturer part Number"
	1    10650 1000
	1    0    0    -1  
$EndComp
$Sheet
S 7800 600  2500 5800
U 5E1D533C
F0 "Leg Board" 50
F1 "LegBoard.sch" 50
F2 "CurlValve" I L 7800 1450 50 
F3 "CurlSensor" I L 7800 2050 50 
F4 "LiftValve" I L 7800 2800 50 
F5 "LiftSensor" I L 7800 3400 50 
F6 "SwingValve" I L 7800 4250 50 
F7 "SwingSensor" I L 7800 4850 50 
F8 "CommB" I R 10300 6000 50 
F9 "CommA" I R 10300 1000 50 
F10 "Power" I L 7800 5750 50 
$EndSheet
$Comp
L Connector_Generic:Conn_02x02_Odd_Even J?
U 1 1 5E1D8173
P 10700 6000
F 0 "J?" H 10750 6307 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 10750 6216 50  0000 C CNN
F 2 "" H 10700 6000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250400/WM1784-ND/252497" H 10700 6000 50  0001 C CNN
F 4 "DigiKey" H 10700 6000 50  0001 C CNN "Supplier"
F 5 "WM1784-ND" H 10750 6125 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 10700 6000 50  0001 C CNN "Manufacturer"
F 7 "0430250400" H 10700 6000 50  0001 C CNN "Manufacturer part Number"
	1    10700 6000
	1    0    0    -1  
$EndComp
$Comp
L Stomp:AngleSensor U?
U 1 1 5E1C2B3D
P 6150 2050
F 0 "U?" H 6117 2555 50  0000 C CNN
F 1 "AngleSensor" H 6117 2464 50  0000 C CNN
F 2 "" H 5700 1800 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5700 1800 50  0001 C CNN
F 4 "TT Electronics" H 6150 2050 50  0001 C CNN "Manufacturer"
F 5 "6127V1A60L.5" H 6117 2373 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 6150 2050 50  0001 C CNN "Supplier"
F 7 "858-6127V1A60L.5" H 6150 2050 50  0001 C CNN "Supplier Part Number"
	1    6150 2050
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E1ECBE8
P 1500 2800
F 0 "SV?" H 1467 3465 50  0000 C CNN
F 1 "Enfield S2" H 1467 3374 50  0000 C CNN
F 2 "" H 700 2500 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 700 2500 50  0001 C CNN
	1    1500 2800
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E1F94F6
P 1500 4250
F 0 "SV?" H 1467 4915 50  0000 C CNN
F 1 "Enfield S2" H 1467 4824 50  0000 C CNN
F 2 "" H 700 3950 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 700 3950 50  0001 C CNN
	1    1500 4250
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:AngleSensor U?
U 1 1 5E20D64D
P 6200 3400
F 0 "U?" H 6167 3905 50  0000 C CNN
F 1 "AngleSensor" H 6167 3814 50  0000 C CNN
F 2 "" H 5750 3150 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5750 3150 50  0001 C CNN
F 4 "TT Electronics" H 6200 3400 50  0001 C CNN "Manufacturer"
F 5 "6127V1A45L.5" H 6167 3723 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 6200 3400 50  0001 C CNN "Supplier"
F 7 "858-6127V1A45L.5" H 6200 3400 50  0001 C CNN "Supplier Part Number"
	1    6200 3400
	-1   0    0    -1  
$EndComp
Text Notes 550  1400 0    197  ~ 0
Curl
Text Notes 600  2950 0    197  ~ 0
Lift
Text Notes 550  4250 0    197  ~ 0
Swing
Text Notes 5250 2050 0    50   ~ 0
60 Degree sensor
$Comp
L Stomp:AngleSensor U?
U 1 1 5E216432
P 6200 4850
F 0 "U?" H 6167 5355 50  0000 C CNN
F 1 "AngleSensor" H 6167 5264 50  0000 C CNN
F 2 "" H 5750 4600 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5750 4600 50  0001 C CNN
F 4 "TT Electronics" H 6200 4850 50  0001 C CNN "Manufacturer"
F 5 "6127V1A45L.5" H 6167 5173 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 6200 4850 50  0001 C CNN "Supplier"
F 7 "858-6127V1A45L.5" H 6200 4850 50  0001 C CNN "Supplier Part Number"
	1    6200 4850
	-1   0    0    -1  
$EndComp
Text Notes 5150 1550 0    50   ~ 0
Micro-Fit Male Pins 26-30AWG, Gold\nMolex 0430310005\nDigi-Key WM3246CT-ND
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E216DD2
P 7150 2800
F 0 "J?" H 7200 3207 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7200 3116 50  0000 C CNN
F 2 "" H 7150 2800 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430200601/WM2762-ND/1132438" H 7150 2800 50  0001 C CNN
F 4 "Digi-Key" H 7150 2800 50  0001 C CNN "Supplier"
F 5 " WM2762-ND " H 7200 3025 50  0000 C CNN "Supplier Part Number"
	1    7150 2800
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E216DE0
P 7100 3400
F 0 "J?" H 7208 3681 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7208 3590 50  0000 C CNN
F 2 "" H 7100 3400 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 7100 3400 50  0001 C CNN
F 4 "Molex" H 7100 3400 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 7100 3400 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 7100 3400 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 7100 3400 50  0001 C CNN "Supplier Part Number"
	1    7100 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E2191A0
P 7150 4250
F 0 "J?" H 7200 4657 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7200 4566 50  0000 C CNN
F 2 "" H 7150 4250 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430200601/WM2762-ND/1132438" H 7150 4250 50  0001 C CNN
F 4 "Digi-Key" H 7150 4250 50  0001 C CNN "Supplier"
F 5 " WM2762-ND " H 7200 4475 50  0000 C CNN "Supplier Part Number"
	1    7150 4250
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E2191AE
P 7100 4850
F 0 "J?" H 7208 5131 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 7208 5040 50  0000 C CNN
F 2 "" H 7100 4850 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 7100 4850 50  0001 C CNN
F 4 "Molex" H 7100 4850 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 7100 4850 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 7100 4850 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 7100 4850 50  0001 C CNN "Supplier Part Number"
	1    7100 4850
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E24883E
P 2600 1250
F 0 "J?" H 2600 975 50  0000 C CNN
F 1 "M8 5 Pin" H 2600 884 50  0000 C CNN
F 2 "" H 2600 1250 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 2600 1250 50  0001 C CNN
F 4 "McMaster-Carr" H 2600 1250 50  0001 C CNN "Supplier"
F 5 "7138K39" H 2600 1250 50  0001 C CNN "Supplier Part Number"
	1    2600 1250
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E2490C7
P 2600 2700
F 0 "J?" H 2600 2425 50  0000 C CNN
F 1 "M8 5 Pin" H 2600 2334 50  0000 C CNN
F 2 "" H 2600 2700 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 2600 2700 50  0001 C CNN
F 4 "McMaster-Carr" H 2600 2700 50  0001 C CNN "Supplier"
F 5 "7138K39" H 2600 2700 50  0001 C CNN "Supplier Part Number"
	1    2600 2700
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E24A55F
P 2600 4150
F 0 "J?" H 2600 3875 50  0000 C CNN
F 1 "M8 5 Pin" H 2600 3784 50  0000 C CNN
F 2 "" H 2600 4150 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 2600 4150 50  0001 C CNN
F 4 "McMaster-Carr" H 2600 4150 50  0001 C CNN "Supplier"
F 5 "7138K39" H 2600 4150 50  0001 C CNN "Supplier Part Number"
	1    2600 4150
	1    0    0    -1  
$EndComp
Text Notes 3050 1250 0    50   ~ 0
Comes with 6ft cable
Text Notes 4700 3500 0    50   ~ 0
Sensor cable is\n26AWG 3 conductor\nCooner Wire NMEF 3/26-6544 J
Text Notes 10900 4450 1    50   ~ 0
Comms\n26AWG 4 conductor\nCooner Wire NMEF 4/26-6544 J
Text Notes 5400 6000 0    50   ~ 0
Power\n20AWG 2 Conductor\nCooner Wire NMEF 2/20-25944 J
$Comp
L Connector_Generic:Conn_01x02 J?
U 1 1 5E24D156
P 7100 5800
F 0 "J?" H 7180 5837 50  0000 L CNN
F 1 "Micro-Fit 3.0 Plug" H 7180 5746 50  0000 L CNN
F 2 "" H 7100 5800 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436450208/WM11226-ND/4247418" H 7100 5800 50  0001 C CNN
F 4 "WM10658-ND" H 7180 5655 50  0000 L CNN "Supplier part Number"
F 5 "Digi-Key" H 7100 5800 50  0001 C CNN "Supplier"
F 6 "Molex" H 7100 5800 50  0001 C CNN "manufacturer"
F 7 "0436450208" H 7100 5800 50  0001 C CNN "Manufacturer part Number"
	1    7100 5800
	1    0    0    -1  
$EndComp
$EndSCHEMATC
