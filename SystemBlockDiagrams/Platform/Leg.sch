EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 8 16
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
U 1 1 5E255C4B
P 1950 1400
AR Path="/5E255C4B" Ref="SV?"  Part="1" 
AR Path="/5E24C641/5E255C4B" Ref="SV1"  Part="1" 
AR Path="/5E264FF3/5E255C4B" Ref="SV4"  Part="1" 
AR Path="/5E2661DC/5E255C4B" Ref="SV7"  Part="1" 
AR Path="/5E269D38/5E255C4B" Ref="SV10"  Part="1" 
AR Path="/5E26B65F/5E255C4B" Ref="SV13"  Part="1" 
AR Path="/5E26CFEA/5E255C4B" Ref="SV16"  Part="1" 
F 0 "SV16" H 1917 2065 50  0000 C CNN
F 1 "Enfield S2" H 1917 1974 50  0000 C CNN
F 2 "" H 1150 1100 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 1150 1100 50  0001 C CNN
F 4 "Enfield" H 1950 1400 50  0001 C CNN "Manufacturer"
F 5 "S2" H 1950 1400 50  0001 C CNN "Manufacturer Part Number"
F 6 "Enfield" H 1950 1400 50  0001 C CNN "Supplier"
F 7 "S2" H 1950 1400 50  0001 C CNN "Supplier Part Number"
	1    1950 1400
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E255C55
P 6550 1000
AR Path="/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255C55" Ref="J23"  Part="1" 
AR Path="/5E264FF3/5E255C55" Ref="J41"  Part="1" 
AR Path="/5E2661DC/5E255C55" Ref="J59"  Part="1" 
AR Path="/5E269D38/5E255C55" Ref="J77"  Part="1" 
AR Path="/5E26B65F/5E255C55" Ref="J95"  Part="1" 
AR Path="/5E26CFEA/5E255C55" Ref="J113"  Part="1" 
F 0 "J113" H 6600 1407 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6600 1316 50  0000 C CNN
F 2 "" H 6550 1000 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250600/WM1785-ND/252498" H 6550 1000 50  0001 C CNN
F 4 "Digi-Key" H 6550 1000 50  0001 C CNN "Supplier"
F 5 "WM1785-ND " H 6600 1225 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 6550 1000 50  0001 C CNN "Manufacturer"
F 7 "0430250600" H 6550 1000 50  0001 C CNN "Manufacturer Part Number"
	1    6550 1000
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E255C60
P 6450 1850
AR Path="/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255C60" Ref="J22"  Part="1" 
AR Path="/5E264FF3/5E255C60" Ref="J40"  Part="1" 
AR Path="/5E2661DC/5E255C60" Ref="J58"  Part="1" 
AR Path="/5E269D38/5E255C60" Ref="J76"  Part="1" 
AR Path="/5E26B65F/5E255C60" Ref="J94"  Part="1" 
AR Path="/5E26CFEA/5E255C60" Ref="J112"  Part="1" 
F 0 "J112" H 6558 2131 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6558 2040 50  0000 C CNN
F 2 "" H 6450 1850 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 6450 1850 50  0001 C CNN
F 4 "Molex" H 6450 1850 50  0001 C CNN "Manufacturer"
F 5 "0436400301" H 6450 1850 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6450 1850 50  0001 C CNN "Supplier"
F 7 " WM1856-ND " H 6550 1650 50  0000 C CNN "Supplier Part Number"
	1    6450 1850
	1    0    0    -1  
$EndComp
$Sheet
S 7100 650  2500 5800
U 5E255C6B
F0 "sheet5E255C45" 50
F1 "LegBoard.sch" 50
F2 "CurlValve" I L 7100 1500 50 
F3 "CurlSensor" I L 7100 2100 50 
F4 "LiftValve" I L 7100 2850 50 
F5 "LiftSensor" I L 7100 3450 50 
F6 "SwingValve" I L 7100 4300 50 
F7 "SwingSensor" I L 7100 4900 50 
F8 "CommDownstream" I R 9600 6050 50 
F9 "CommUpstream" I R 9600 1050 50 
F10 "Power" I L 7100 5800 50 
$EndSheet
$Comp
L Stomp:AngleSensor U?
U 1 1 5E255C75
P 5550 1850
AR Path="/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E24C641/5E255C75" Ref="U1"  Part="1" 
AR Path="/5E264FF3/5E255C75" Ref="U4"  Part="1" 
AR Path="/5E2661DC/5E255C75" Ref="U7"  Part="1" 
AR Path="/5E269D38/5E255C75" Ref="U10"  Part="1" 
AR Path="/5E26B65F/5E255C75" Ref="U13"  Part="1" 
AR Path="/5E26CFEA/5E255C75" Ref="U16"  Part="1" 
F 0 "U16" H 5517 2355 50  0000 C CNN
F 1 "AngleSensor" H 5517 2264 50  0000 C CNN
F 2 "" H 5100 1600 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5100 1600 50  0001 C CNN
F 4 "TT Electronics" H 5550 1850 50  0001 C CNN "Manufacturer"
F 5 "6127V1A60L.5" H 5517 2173 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 5550 1850 50  0001 C CNN "Supplier"
F 7 "858-6127V1A60L.5" H 5550 1850 50  0001 C CNN "Supplier Part Number"
	1    5550 1850
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:AngleSensor U?
U 1 1 5E255C8B
P 5200 3800
AR Path="/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E24C641/5E255C8B" Ref="U2"  Part="1" 
AR Path="/5E264FF3/5E255C8B" Ref="U5"  Part="1" 
AR Path="/5E2661DC/5E255C8B" Ref="U8"  Part="1" 
AR Path="/5E269D38/5E255C8B" Ref="U11"  Part="1" 
AR Path="/5E26B65F/5E255C8B" Ref="U14"  Part="1" 
AR Path="/5E26CFEA/5E255C8B" Ref="U17"  Part="1" 
F 0 "U17" H 5167 4305 50  0000 C CNN
F 1 "AngleSensor" H 5167 4214 50  0000 C CNN
F 2 "" H 4750 3550 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 4750 3550 50  0001 C CNN
F 4 "TT Electronics" H 5200 3800 50  0001 C CNN "Manufacturer"
F 5 "6127V1A45L.5" H 5167 4123 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 5200 3800 50  0001 C CNN "Supplier"
F 7 "858-6127V1A45L.5" H 5200 3800 50  0001 C CNN "Supplier Part Number"
	1    5200 3800
	-1   0    0    -1  
$EndComp
Text Notes 1000 1450 0    197  ~ 0
Curl
Text Notes 1050 3000 0    197  ~ 0
Lift
Text Notes 1000 4300 0    197  ~ 0
Swing
Text Notes 5400 2200 0    50   ~ 0
60 Degree sensor
$Comp
L Stomp:AngleSensor U?
U 1 1 5E255C99
P 5400 5750
AR Path="/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E24C641/5E255C99" Ref="U3"  Part="1" 
AR Path="/5E264FF3/5E255C99" Ref="U6"  Part="1" 
AR Path="/5E2661DC/5E255C99" Ref="U9"  Part="1" 
AR Path="/5E269D38/5E255C99" Ref="U12"  Part="1" 
AR Path="/5E26B65F/5E255C99" Ref="U15"  Part="1" 
AR Path="/5E26CFEA/5E255C99" Ref="U18"  Part="1" 
F 0 "U18" H 5367 6255 50  0000 C CNN
F 1 "AngleSensor" H 5367 6164 50  0000 C CNN
F 2 "" H 4950 5500 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 4950 5500 50  0001 C CNN
F 4 "TT Electronics" H 5400 5750 50  0001 C CNN "Manufacturer"
F 5 "6127V1A45L.5" H 5367 6073 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 5400 5750 50  0001 C CNN "Supplier"
F 7 "858-6127V1A45L.5" H 5400 5750 50  0001 C CNN "Supplier Part Number"
	1    5400 5750
	-1   0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E255CA4
P 6100 3800
AR Path="/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CA4" Ref="J25"  Part="1" 
AR Path="/5E264FF3/5E255CA4" Ref="J43"  Part="1" 
AR Path="/5E2661DC/5E255CA4" Ref="J61"  Part="1" 
AR Path="/5E269D38/5E255CA4" Ref="J79"  Part="1" 
AR Path="/5E26B65F/5E255CA4" Ref="J97"  Part="1" 
AR Path="/5E26CFEA/5E255CA4" Ref="J115"  Part="1" 
F 0 "J115" H 6208 4081 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6208 3990 50  0000 C CNN
F 2 "" H 6100 3800 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 6100 3800 50  0001 C CNN
F 4 "Molex" H 6100 3800 50  0001 C CNN "Manufacturer"
F 5 "0436400301" H 6100 3800 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6100 3800 50  0001 C CNN "Supplier"
F 7 " WM1856-ND " H 6100 3800 50  0001 C CNN "Supplier Part Number"
	1    6100 3800
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E255CAE
P 6300 5750
AR Path="/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CAE" Ref="J26"  Part="1" 
AR Path="/5E264FF3/5E255CAE" Ref="J44"  Part="1" 
AR Path="/5E2661DC/5E255CAE" Ref="J62"  Part="1" 
AR Path="/5E269D38/5E255CAE" Ref="J80"  Part="1" 
AR Path="/5E26B65F/5E255CAE" Ref="J98"  Part="1" 
AR Path="/5E26CFEA/5E255CAE" Ref="J116"  Part="1" 
F 0 "J116" H 6408 6031 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6408 5940 50  0000 C CNN
F 2 "" H 6300 5750 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 6300 5750 50  0001 C CNN
F 4 "Molex" H 6300 5750 50  0001 C CNN "Manufacturer"
F 5 "0436400301" H 6300 5750 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6300 5750 50  0001 C CNN "Supplier"
F 7 " WM1856-ND " H 6300 5750 50  0001 C CNN "Supplier Part Number"
	1    6300 5750
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E255CB6
P 3050 1300
AR Path="/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CB6" Ref="J19"  Part="1" 
AR Path="/5E264FF3/5E255CB6" Ref="J37"  Part="1" 
AR Path="/5E2661DC/5E255CB6" Ref="J55"  Part="1" 
AR Path="/5E269D38/5E255CB6" Ref="J73"  Part="1" 
AR Path="/5E26B65F/5E255CB6" Ref="J91"  Part="1" 
AR Path="/5E26CFEA/5E255CB6" Ref="J109"  Part="1" 
F 0 "J109" H 3050 1025 50  0000 C CNN
F 1 "M8 5 Pin" H 3050 934 50  0000 C CNN
F 2 "" H 3050 1300 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 3050 1300 50  0001 C CNN
F 4 "McMaster-Carr" H 3050 1300 50  0001 C CNN "Supplier"
F 5 "7138K39" H 3050 1300 50  0001 C CNN "Supplier Part Number"
	1    3050 1300
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E255CBE
P 3050 2750
AR Path="/5E255CBE" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CBE" Ref="J20"  Part="1" 
AR Path="/5E264FF3/5E255CBE" Ref="J38"  Part="1" 
AR Path="/5E2661DC/5E255CBE" Ref="J56"  Part="1" 
AR Path="/5E269D38/5E255CBE" Ref="J74"  Part="1" 
AR Path="/5E26B65F/5E255CBE" Ref="J92"  Part="1" 
AR Path="/5E26CFEA/5E255CBE" Ref="J110"  Part="1" 
F 0 "J110" H 3050 2475 50  0000 C CNN
F 1 "M8 5 Pin" H 3050 2384 50  0000 C CNN
F 2 "" H 3050 2750 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 3050 2750 50  0001 C CNN
F 4 "McMaster-Carr" H 3050 2750 50  0001 C CNN "Supplier"
F 5 "7138K39" H 3050 2750 50  0001 C CNN "Supplier Part Number"
	1    3050 2750
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E255CC6
P 3050 4200
AR Path="/5E255CC6" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CC6" Ref="J21"  Part="1" 
AR Path="/5E264FF3/5E255CC6" Ref="J39"  Part="1" 
AR Path="/5E2661DC/5E255CC6" Ref="J57"  Part="1" 
AR Path="/5E269D38/5E255CC6" Ref="J75"  Part="1" 
AR Path="/5E26B65F/5E255CC6" Ref="J93"  Part="1" 
AR Path="/5E26CFEA/5E255CC6" Ref="J111"  Part="1" 
F 0 "J111" H 3050 3925 50  0000 C CNN
F 1 "M8 5 Pin" H 3050 3834 50  0000 C CNN
F 2 "" H 3050 4200 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 3050 4200 50  0001 C CNN
F 4 "McMaster-Carr" H 3050 4200 50  0001 C CNN "Supplier"
F 5 "7138K39" H 3050 4200 50  0001 C CNN "Supplier Part Number"
	1    3050 4200
	1    0    0    -1  
$EndComp
Text Notes 2800 900  0    50   ~ 0
Comes with 6ft cable
Text Notes 850  6150 0    50   ~ 0
Sensor cable is\n26AWG 3 conductor\nCooner Wire NMEF 3/26-6544 J
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E255CD2
P 6150 2900
AR Path="/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CD2" Ref="J24"  Part="1" 
AR Path="/5E264FF3/5E255CD2" Ref="J42"  Part="1" 
AR Path="/5E2661DC/5E255CD2" Ref="J60"  Part="1" 
AR Path="/5E269D38/5E255CD2" Ref="J78"  Part="1" 
AR Path="/5E26B65F/5E255CD2" Ref="J96"  Part="1" 
AR Path="/5E26CFEA/5E255CD2" Ref="J114"  Part="1" 
F 0 "J114" H 6200 3307 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6200 3216 50  0000 C CNN
F 2 "" H 6150 2900 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250600/WM1785-ND/252498" H 6150 2900 50  0001 C CNN
F 4 "Digi-Key" H 6150 2900 50  0001 C CNN "Supplier"
F 5 "WM1785-ND " H 6200 3125 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 6150 2900 50  0001 C CNN "Manufacturer"
F 7 "0430250600" H 6150 2900 50  0001 C CNN "Manufacturer Part Number"
	1    6150 2900
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E255CDC
P 6200 4700
AR Path="/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CDC" Ref="J27"  Part="1" 
AR Path="/5E264FF3/5E255CDC" Ref="J45"  Part="1" 
AR Path="/5E2661DC/5E255CDC" Ref="J63"  Part="1" 
AR Path="/5E269D38/5E255CDC" Ref="J81"  Part="1" 
AR Path="/5E26B65F/5E255CDC" Ref="J99"  Part="1" 
AR Path="/5E26CFEA/5E255CDC" Ref="J117"  Part="1" 
F 0 "J117" H 6250 5107 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6250 5016 50  0000 C CNN
F 2 "" H 6200 4700 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250600/WM1785-ND/252498" H 6200 4700 50  0001 C CNN
F 4 "Digi-Key" H 6200 4700 50  0001 C CNN "Supplier"
F 5 "WM1785-ND " H 6250 4925 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 6200 4700 50  0001 C CNN "Manufacturer"
F 7 "0430250600" H 6200 4700 50  0001 C CNN "Manufacturer Part Number"
	1    6200 4700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E48C4D1
P 1950 3000
AR Path="/5E48C4D1" Ref="SV?"  Part="1" 
AR Path="/5E24C641/5E48C4D1" Ref="SV2"  Part="1" 
AR Path="/5E264FF3/5E48C4D1" Ref="SV6"  Part="1" 
AR Path="/5E2661DC/5E48C4D1" Ref="SV9"  Part="1" 
AR Path="/5E269D38/5E48C4D1" Ref="SV12"  Part="1" 
AR Path="/5E26B65F/5E48C4D1" Ref="SV15"  Part="1" 
AR Path="/5E26CFEA/5E48C4D1" Ref="SV18"  Part="1" 
F 0 "SV18" H 1917 3665 50  0000 C CNN
F 1 "Enfield S2" H 1917 3574 50  0000 C CNN
F 2 "" H 1150 2700 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 1150 2700 50  0001 C CNN
F 4 "Enfield" H 1950 3000 50  0001 C CNN "Manufacturer"
F 5 "S2" H 1950 3000 50  0001 C CNN "Manufacturer Part Number"
F 6 "Enfield" H 1950 3000 50  0001 C CNN "Supplier"
F 7 "S2" H 1950 3000 50  0001 C CNN "Supplier Part Number"
	1    1950 3000
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E48CEF4
P 1900 4350
AR Path="/5E48CEF4" Ref="SV?"  Part="1" 
AR Path="/5E24C641/5E48CEF4" Ref="SV3"  Part="1" 
AR Path="/5E264FF3/5E48CEF4" Ref="SV5"  Part="1" 
AR Path="/5E2661DC/5E48CEF4" Ref="SV8"  Part="1" 
AR Path="/5E269D38/5E48CEF4" Ref="SV11"  Part="1" 
AR Path="/5E26B65F/5E48CEF4" Ref="SV14"  Part="1" 
AR Path="/5E26CFEA/5E48CEF4" Ref="SV17"  Part="1" 
F 0 "SV17" H 1867 5015 50  0000 C CNN
F 1 "Enfield S2" H 1867 4924 50  0000 C CNN
F 2 "" H 1100 4050 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 1100 4050 50  0001 C CNN
F 4 "Enfield" H 1900 4350 50  0001 C CNN "Manufacturer"
F 5 "S2" H 1900 4350 50  0001 C CNN "Manufacturer Part Number"
F 6 "Enfield" H 1900 4350 50  0001 C CNN "Supplier"
F 7 "S2" H 1900 4350 50  0001 C CNN "Supplier Part Number"
	1    1900 4350
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN67
U 1 1 5E4B1380
P 4000 3700
AR Path="/5E24C641/5E4B1380" Ref="PN67"  Part="1" 
AR Path="/5E269D38/5E4B1380" Ref="PN148"  Part="1" 
AR Path="/5E264FF3/5E4B1380" Ref="PN94"  Part="1" 
AR Path="/5E2661DC/5E4B1380" Ref="PN121"  Part="1" 
AR Path="/5E26B65F/5E4B1380" Ref="PN175"  Part="1" 
AR Path="/5E26CFEA/5E4B1380" Ref="PN202"  Part="1" 
F 0 "PN202" H 3950 3850 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 3950 3800 50  0000 L CNN
F 2 "" H 4000 3700 50  0001 C CNN
F 3 "" H 4000 3700 50  0001 C CNN
F 4 "Molex" H 4100 3700 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4350 3700 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4000 3700 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4000 3700 50  0001 C CNN "Supplier Part Number"
	1    4000 3700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN68
U 1 1 5E4B5BFA
P 4000 3900
AR Path="/5E24C641/5E4B5BFA" Ref="PN68"  Part="1" 
AR Path="/5E269D38/5E4B5BFA" Ref="PN149"  Part="1" 
AR Path="/5E264FF3/5E4B5BFA" Ref="PN95"  Part="1" 
AR Path="/5E2661DC/5E4B5BFA" Ref="PN122"  Part="1" 
AR Path="/5E26B65F/5E4B5BFA" Ref="PN176"  Part="1" 
AR Path="/5E26CFEA/5E4B5BFA" Ref="PN203"  Part="1" 
F 0 "PN203" H 3950 4050 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 3950 4000 50  0000 L CNN
F 2 "" H 4000 3900 50  0001 C CNN
F 3 "" H 4000 3900 50  0001 C CNN
F 4 "Molex" H 4100 3900 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4350 3900 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4000 3900 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4000 3900 50  0001 C CNN "Supplier Part Number"
	1    4000 3900
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN69
U 1 1 5E4B6083
P 4000 4100
AR Path="/5E24C641/5E4B6083" Ref="PN69"  Part="1" 
AR Path="/5E269D38/5E4B6083" Ref="PN150"  Part="1" 
AR Path="/5E264FF3/5E4B6083" Ref="PN96"  Part="1" 
AR Path="/5E2661DC/5E4B6083" Ref="PN123"  Part="1" 
AR Path="/5E26B65F/5E4B6083" Ref="PN177"  Part="1" 
AR Path="/5E26CFEA/5E4B6083" Ref="PN204"  Part="1" 
F 0 "PN204" H 3950 4250 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 3950 4200 50  0000 L CNN
F 2 "" H 4000 4100 50  0001 C CNN
F 3 "" H 4000 4100 50  0001 C CNN
F 4 "Molex" H 4100 4100 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4350 4100 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4000 4100 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4000 4100 50  0001 C CNN "Supplier Part Number"
	1    4000 4100
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN73
U 1 1 5E4BB919
P 4200 5600
AR Path="/5E24C641/5E4BB919" Ref="PN73"  Part="1" 
AR Path="/5E269D38/5E4BB919" Ref="PN154"  Part="1" 
AR Path="/5E264FF3/5E4BB919" Ref="PN100"  Part="1" 
AR Path="/5E2661DC/5E4BB919" Ref="PN127"  Part="1" 
AR Path="/5E26B65F/5E4BB919" Ref="PN181"  Part="1" 
AR Path="/5E26CFEA/5E4BB919" Ref="PN208"  Part="1" 
F 0 "PN208" H 4150 5750 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 4150 5700 50  0000 L CNN
F 2 "" H 4200 5600 50  0001 C CNN
F 3 "" H 4200 5600 50  0001 C CNN
F 4 "Molex" H 4300 5600 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4550 5600 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4200 5600 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4200 5600 50  0001 C CNN "Supplier Part Number"
	1    4200 5600
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN74
U 1 1 5E4BB923
P 4200 5800
AR Path="/5E24C641/5E4BB923" Ref="PN74"  Part="1" 
AR Path="/5E269D38/5E4BB923" Ref="PN155"  Part="1" 
AR Path="/5E264FF3/5E4BB923" Ref="PN101"  Part="1" 
AR Path="/5E2661DC/5E4BB923" Ref="PN128"  Part="1" 
AR Path="/5E26B65F/5E4BB923" Ref="PN182"  Part="1" 
AR Path="/5E26CFEA/5E4BB923" Ref="PN209"  Part="1" 
F 0 "PN209" H 4150 5950 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 4150 5900 50  0000 L CNN
F 2 "" H 4200 5800 50  0001 C CNN
F 3 "" H 4200 5800 50  0001 C CNN
F 4 "Molex" H 4300 5800 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4550 5800 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4200 5800 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4200 5800 50  0001 C CNN "Supplier Part Number"
	1    4200 5800
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN75
U 1 1 5E4BB92D
P 4200 6000
AR Path="/5E24C641/5E4BB92D" Ref="PN75"  Part="1" 
AR Path="/5E269D38/5E4BB92D" Ref="PN156"  Part="1" 
AR Path="/5E264FF3/5E4BB92D" Ref="PN102"  Part="1" 
AR Path="/5E2661DC/5E4BB92D" Ref="PN129"  Part="1" 
AR Path="/5E26B65F/5E4BB92D" Ref="PN183"  Part="1" 
AR Path="/5E26CFEA/5E4BB92D" Ref="PN210"  Part="1" 
F 0 "PN210" H 4150 6150 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 4150 6100 50  0000 L CNN
F 2 "" H 4200 6000 50  0001 C CNN
F 3 "" H 4200 6000 50  0001 C CNN
F 4 "Molex" H 4300 6000 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4550 6000 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4200 6000 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4200 6000 50  0001 C CNN "Supplier Part Number"
	1    4200 6000
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN76
U 1 1 5E4C0438
P 4400 1650
AR Path="/5E24C641/5E4C0438" Ref="PN76"  Part="1" 
AR Path="/5E269D38/5E4C0438" Ref="PN157"  Part="1" 
AR Path="/5E264FF3/5E4C0438" Ref="PN103"  Part="1" 
AR Path="/5E2661DC/5E4C0438" Ref="PN130"  Part="1" 
AR Path="/5E26B65F/5E4C0438" Ref="PN184"  Part="1" 
AR Path="/5E26CFEA/5E4C0438" Ref="PN211"  Part="1" 
F 0 "PN211" H 4350 1800 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 4350 1750 50  0000 L CNN
F 2 "" H 4400 1650 50  0001 C CNN
F 3 "" H 4400 1650 50  0001 C CNN
F 4 "Molex" H 4500 1650 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4750 1650 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4400 1650 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4400 1650 50  0001 C CNN "Supplier Part Number"
	1    4400 1650
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN77
U 1 1 5E4C0442
P 4400 1850
AR Path="/5E24C641/5E4C0442" Ref="PN77"  Part="1" 
AR Path="/5E269D38/5E4C0442" Ref="PN158"  Part="1" 
AR Path="/5E264FF3/5E4C0442" Ref="PN104"  Part="1" 
AR Path="/5E2661DC/5E4C0442" Ref="PN131"  Part="1" 
AR Path="/5E26B65F/5E4C0442" Ref="PN185"  Part="1" 
AR Path="/5E26CFEA/5E4C0442" Ref="PN212"  Part="1" 
F 0 "PN212" H 4350 2000 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 4350 1950 50  0000 L CNN
F 2 "" H 4400 1850 50  0001 C CNN
F 3 "" H 4400 1850 50  0001 C CNN
F 4 "Molex" H 4500 1850 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4750 1850 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4400 1850 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4400 1850 50  0001 C CNN "Supplier Part Number"
	1    4400 1850
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN78
U 1 1 5E4C044C
P 4400 2050
AR Path="/5E24C641/5E4C044C" Ref="PN78"  Part="1" 
AR Path="/5E269D38/5E4C044C" Ref="PN159"  Part="1" 
AR Path="/5E264FF3/5E4C044C" Ref="PN105"  Part="1" 
AR Path="/5E2661DC/5E4C044C" Ref="PN132"  Part="1" 
AR Path="/5E26B65F/5E4C044C" Ref="PN186"  Part="1" 
AR Path="/5E26CFEA/5E4C044C" Ref="PN213"  Part="1" 
F 0 "PN213" H 4350 2200 50  0000 L CNN
F 1 "Female Pin 26-30AWG" H 4350 2150 50  0000 L CNN
F 2 "" H 4400 2050 50  0001 C CNN
F 3 "" H 4400 2050 50  0001 C CNN
F 4 "Molex" H 4500 2050 50  0000 L CNN "Manufacturer"
F 5 "0430300005" H 4750 2050 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4400 2050 50  0001 C CNN "Supplier"
F 7 "WM9169CT-ND" H 4400 2050 50  0001 C CNN "Supplier Part Number"
	1    4400 2050
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN167
U 1 1 5E54972B
P 5150 900
AR Path="/5E269D38/5E54972B" Ref="PN167"  Part="1" 
AR Path="/5E24C641/5E54972B" Ref="PN86"  Part="1" 
AR Path="/5E264FF3/5E54972B" Ref="PN113"  Part="1" 
AR Path="/5E2661DC/5E54972B" Ref="PN140"  Part="1" 
AR Path="/5E26B65F/5E54972B" Ref="PN194"  Part="1" 
AR Path="/5E26CFEA/5E54972B" Ref="PN221"  Part="1" 
F 0 "PN221" H 5100 1000 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 5300 1000 50  0000 L CNN
F 2 "" H 5150 900 50  0001 C CNN
F 3 "" H 5150 900 50  0001 C CNN
F 4 "Molex" H 5250 900 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5500 900 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 5150 900 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 5150 900 50  0001 C CNN "Supplier Part Number"
	1    5150 900 
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN168
U 1 1 5E54AC3D
P 5150 1100
AR Path="/5E269D38/5E54AC3D" Ref="PN168"  Part="1" 
AR Path="/5E24C641/5E54AC3D" Ref="PN87"  Part="1" 
AR Path="/5E264FF3/5E54AC3D" Ref="PN114"  Part="1" 
AR Path="/5E2661DC/5E54AC3D" Ref="PN141"  Part="1" 
AR Path="/5E26B65F/5E54AC3D" Ref="PN195"  Part="1" 
AR Path="/5E26CFEA/5E54AC3D" Ref="PN222"  Part="1" 
F 0 "PN222" H 5100 1200 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 5300 1200 50  0000 L CNN
F 2 "" H 5150 1100 50  0001 C CNN
F 3 "" H 5150 1100 50  0001 C CNN
F 4 "Molex" H 5250 1100 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5500 1100 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 5150 1100 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 5150 1100 50  0001 C CNN "Supplier Part Number"
	1    5150 1100
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN151
U 1 1 5E54BC0E
P 4150 700
AR Path="/5E269D38/5E54BC0E" Ref="PN151"  Part="1" 
AR Path="/5E24C641/5E54BC0E" Ref="PN70"  Part="1" 
AR Path="/5E264FF3/5E54BC0E" Ref="PN97"  Part="1" 
AR Path="/5E2661DC/5E54BC0E" Ref="PN124"  Part="1" 
AR Path="/5E26B65F/5E54BC0E" Ref="PN178"  Part="1" 
AR Path="/5E26CFEA/5E54BC0E" Ref="PN205"  Part="1" 
F 0 "PN205" H 4100 800 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4300 800 50  0000 L CNN
F 2 "" H 4150 700 50  0001 C CNN
F 3 "" H 4150 700 50  0001 C CNN
F 4 "Molex" H 4250 700 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4500 700 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4150 700 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4150 700 50  0001 C CNN "Supplier Part Number"
	1    4150 700 
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN152
U 1 1 5E54BC1C
P 4150 900
AR Path="/5E269D38/5E54BC1C" Ref="PN152"  Part="1" 
AR Path="/5E24C641/5E54BC1C" Ref="PN71"  Part="1" 
AR Path="/5E264FF3/5E54BC1C" Ref="PN98"  Part="1" 
AR Path="/5E2661DC/5E54BC1C" Ref="PN125"  Part="1" 
AR Path="/5E26B65F/5E54BC1C" Ref="PN179"  Part="1" 
AR Path="/5E26CFEA/5E54BC1C" Ref="PN206"  Part="1" 
F 0 "PN206" H 4100 1000 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4300 1000 50  0000 L CNN
F 2 "" H 4150 900 50  0001 C CNN
F 3 "" H 4150 900 50  0001 C CNN
F 4 "Molex" H 4250 900 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4500 900 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4150 900 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4150 900 50  0001 C CNN "Supplier Part Number"
	1    4150 900 
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN166
U 1 1 5E54E2AE
P 5150 700
AR Path="/5E269D38/5E54E2AE" Ref="PN166"  Part="1" 
AR Path="/5E24C641/5E54E2AE" Ref="PN85"  Part="1" 
AR Path="/5E264FF3/5E54E2AE" Ref="PN112"  Part="1" 
AR Path="/5E2661DC/5E54E2AE" Ref="PN139"  Part="1" 
AR Path="/5E26B65F/5E54E2AE" Ref="PN193"  Part="1" 
AR Path="/5E26CFEA/5E54E2AE" Ref="PN220"  Part="1" 
F 0 "PN220" H 5100 800 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 5300 800 50  0000 L CNN
F 2 "" H 5150 700 50  0001 C CNN
F 3 "" H 5150 700 50  0001 C CNN
F 4 "Molex" H 5250 700 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5500 700 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 5150 700 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 5150 700 50  0001 C CNN "Supplier Part Number"
	1    5150 700 
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN153
U 1 1 5E54E5F5
P 4150 1100
AR Path="/5E269D38/5E54E5F5" Ref="PN153"  Part="1" 
AR Path="/5E24C641/5E54E5F5" Ref="PN72"  Part="1" 
AR Path="/5E264FF3/5E54E5F5" Ref="PN99"  Part="1" 
AR Path="/5E2661DC/5E54E5F5" Ref="PN126"  Part="1" 
AR Path="/5E26B65F/5E54E5F5" Ref="PN180"  Part="1" 
AR Path="/5E26CFEA/5E54E5F5" Ref="PN207"  Part="1" 
F 0 "PN207" H 4100 1200 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4300 1200 50  0000 L CNN
F 2 "" H 4150 1100 50  0001 C CNN
F 3 "" H 4150 1100 50  0001 C CNN
F 4 "Molex" H 4250 1100 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4500 1100 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4150 1100 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4150 1100 50  0001 C CNN "Supplier Part Number"
	1    4150 1100
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN161
U 1 1 5E57CB37
P 4700 4700
AR Path="/5E269D38/5E57CB37" Ref="PN161"  Part="1" 
AR Path="/5E24C641/5E57CB37" Ref="PN80"  Part="1" 
AR Path="/5E264FF3/5E57CB37" Ref="PN107"  Part="1" 
AR Path="/5E2661DC/5E57CB37" Ref="PN134"  Part="1" 
AR Path="/5E26B65F/5E57CB37" Ref="PN188"  Part="1" 
AR Path="/5E26CFEA/5E57CB37" Ref="PN215"  Part="1" 
F 0 "PN215" H 4650 4800 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4850 4800 50  0000 L CNN
F 2 "" H 4700 4700 50  0001 C CNN
F 3 "" H 4700 4700 50  0001 C CNN
F 4 "Molex" H 4800 4700 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5050 4700 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4700 4700 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4700 4700 50  0001 C CNN "Supplier Part Number"
	1    4700 4700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN162
U 1 1 5E57CB45
P 4700 4900
AR Path="/5E269D38/5E57CB45" Ref="PN162"  Part="1" 
AR Path="/5E24C641/5E57CB45" Ref="PN81"  Part="1" 
AR Path="/5E264FF3/5E57CB45" Ref="PN108"  Part="1" 
AR Path="/5E2661DC/5E57CB45" Ref="PN135"  Part="1" 
AR Path="/5E26B65F/5E57CB45" Ref="PN189"  Part="1" 
AR Path="/5E26CFEA/5E57CB45" Ref="PN216"  Part="1" 
F 0 "PN216" H 4650 5000 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4850 5000 50  0000 L CNN
F 2 "" H 4700 4900 50  0001 C CNN
F 3 "" H 4700 4900 50  0001 C CNN
F 4 "Molex" H 4800 4900 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5050 4900 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4700 4900 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4700 4900 50  0001 C CNN "Supplier Part Number"
	1    4700 4900
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN142
U 1 1 5E57CB53
P 3700 4500
AR Path="/5E269D38/5E57CB53" Ref="PN142"  Part="1" 
AR Path="/5E24C641/5E57CB53" Ref="PN61"  Part="1" 
AR Path="/5E264FF3/5E57CB53" Ref="PN88"  Part="1" 
AR Path="/5E2661DC/5E57CB53" Ref="PN115"  Part="1" 
AR Path="/5E26B65F/5E57CB53" Ref="PN169"  Part="1" 
AR Path="/5E26CFEA/5E57CB53" Ref="PN196"  Part="1" 
F 0 "PN196" H 3650 4600 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 3850 4600 50  0000 L CNN
F 2 "" H 3700 4500 50  0001 C CNN
F 3 "" H 3700 4500 50  0001 C CNN
F 4 "Molex" H 3800 4500 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4050 4500 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 3700 4500 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 3700 4500 50  0001 C CNN "Supplier Part Number"
	1    3700 4500
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN143
U 1 1 5E57CB61
P 3700 4700
AR Path="/5E269D38/5E57CB61" Ref="PN143"  Part="1" 
AR Path="/5E24C641/5E57CB61" Ref="PN62"  Part="1" 
AR Path="/5E264FF3/5E57CB61" Ref="PN89"  Part="1" 
AR Path="/5E2661DC/5E57CB61" Ref="PN116"  Part="1" 
AR Path="/5E26B65F/5E57CB61" Ref="PN170"  Part="1" 
AR Path="/5E26CFEA/5E57CB61" Ref="PN197"  Part="1" 
F 0 "PN197" H 3650 4800 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 3850 4800 50  0000 L CNN
F 2 "" H 3700 4700 50  0001 C CNN
F 3 "" H 3700 4700 50  0001 C CNN
F 4 "Molex" H 3800 4700 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4050 4700 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 3700 4700 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 3700 4700 50  0001 C CNN "Supplier Part Number"
	1    3700 4700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN160
U 1 1 5E57CB6F
P 4700 4500
AR Path="/5E269D38/5E57CB6F" Ref="PN160"  Part="1" 
AR Path="/5E24C641/5E57CB6F" Ref="PN79"  Part="1" 
AR Path="/5E264FF3/5E57CB6F" Ref="PN106"  Part="1" 
AR Path="/5E2661DC/5E57CB6F" Ref="PN133"  Part="1" 
AR Path="/5E26B65F/5E57CB6F" Ref="PN187"  Part="1" 
AR Path="/5E26CFEA/5E57CB6F" Ref="PN214"  Part="1" 
F 0 "PN214" H 4650 4600 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4850 4600 50  0000 L CNN
F 2 "" H 4700 4500 50  0001 C CNN
F 3 "" H 4700 4500 50  0001 C CNN
F 4 "Molex" H 4800 4500 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5050 4500 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4700 4500 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4700 4500 50  0001 C CNN "Supplier Part Number"
	1    4700 4500
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN144
U 1 1 5E57CB7D
P 3700 4900
AR Path="/5E269D38/5E57CB7D" Ref="PN144"  Part="1" 
AR Path="/5E24C641/5E57CB7D" Ref="PN63"  Part="1" 
AR Path="/5E264FF3/5E57CB7D" Ref="PN90"  Part="1" 
AR Path="/5E2661DC/5E57CB7D" Ref="PN117"  Part="1" 
AR Path="/5E26B65F/5E57CB7D" Ref="PN171"  Part="1" 
AR Path="/5E26CFEA/5E57CB7D" Ref="PN198"  Part="1" 
F 0 "PN198" H 3650 5000 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 3850 5000 50  0000 L CNN
F 2 "" H 3700 4900 50  0001 C CNN
F 3 "" H 3700 4900 50  0001 C CNN
F 4 "Molex" H 3800 4900 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4050 4900 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 3700 4900 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 3700 4900 50  0001 C CNN "Supplier Part Number"
	1    3700 4900
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN164
U 1 1 5E59408C
P 4750 2850
AR Path="/5E269D38/5E59408C" Ref="PN164"  Part="1" 
AR Path="/5E24C641/5E59408C" Ref="PN83"  Part="1" 
AR Path="/5E264FF3/5E59408C" Ref="PN110"  Part="1" 
AR Path="/5E2661DC/5E59408C" Ref="PN137"  Part="1" 
AR Path="/5E26B65F/5E59408C" Ref="PN191"  Part="1" 
AR Path="/5E26CFEA/5E59408C" Ref="PN218"  Part="1" 
F 0 "PN218" H 4700 2950 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4900 2950 50  0000 L CNN
F 2 "" H 4750 2850 50  0001 C CNN
F 3 "" H 4750 2850 50  0001 C CNN
F 4 "Molex" H 4850 2850 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5100 2850 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4750 2850 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4750 2850 50  0001 C CNN "Supplier Part Number"
	1    4750 2850
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN165
U 1 1 5E59409A
P 4750 3050
AR Path="/5E269D38/5E59409A" Ref="PN165"  Part="1" 
AR Path="/5E24C641/5E59409A" Ref="PN84"  Part="1" 
AR Path="/5E264FF3/5E59409A" Ref="PN111"  Part="1" 
AR Path="/5E2661DC/5E59409A" Ref="PN138"  Part="1" 
AR Path="/5E26B65F/5E59409A" Ref="PN192"  Part="1" 
AR Path="/5E26CFEA/5E59409A" Ref="PN219"  Part="1" 
F 0 "PN219" H 4700 3150 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4900 3150 50  0000 L CNN
F 2 "" H 4750 3050 50  0001 C CNN
F 3 "" H 4750 3050 50  0001 C CNN
F 4 "Molex" H 4850 3050 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5100 3050 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4750 3050 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4750 3050 50  0001 C CNN "Supplier Part Number"
	1    4750 3050
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN145
U 1 1 5E5940A8
P 3750 2650
AR Path="/5E269D38/5E5940A8" Ref="PN145"  Part="1" 
AR Path="/5E24C641/5E5940A8" Ref="PN64"  Part="1" 
AR Path="/5E264FF3/5E5940A8" Ref="PN91"  Part="1" 
AR Path="/5E2661DC/5E5940A8" Ref="PN118"  Part="1" 
AR Path="/5E26B65F/5E5940A8" Ref="PN172"  Part="1" 
AR Path="/5E26CFEA/5E5940A8" Ref="PN199"  Part="1" 
F 0 "PN199" H 3700 2750 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 3900 2750 50  0000 L CNN
F 2 "" H 3750 2650 50  0001 C CNN
F 3 "" H 3750 2650 50  0001 C CNN
F 4 "Molex" H 3850 2650 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4100 2650 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 3750 2650 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 3750 2650 50  0001 C CNN "Supplier Part Number"
	1    3750 2650
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN146
U 1 1 5E5940B6
P 3750 2850
AR Path="/5E269D38/5E5940B6" Ref="PN146"  Part="1" 
AR Path="/5E24C641/5E5940B6" Ref="PN65"  Part="1" 
AR Path="/5E264FF3/5E5940B6" Ref="PN92"  Part="1" 
AR Path="/5E2661DC/5E5940B6" Ref="PN119"  Part="1" 
AR Path="/5E26B65F/5E5940B6" Ref="PN173"  Part="1" 
AR Path="/5E26CFEA/5E5940B6" Ref="PN200"  Part="1" 
F 0 "PN200" H 3700 2950 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 3900 2950 50  0000 L CNN
F 2 "" H 3750 2850 50  0001 C CNN
F 3 "" H 3750 2850 50  0001 C CNN
F 4 "Molex" H 3850 2850 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4100 2850 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 3750 2850 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 3750 2850 50  0001 C CNN "Supplier Part Number"
	1    3750 2850
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN163
U 1 1 5E5940C4
P 4750 2650
AR Path="/5E269D38/5E5940C4" Ref="PN163"  Part="1" 
AR Path="/5E24C641/5E5940C4" Ref="PN82"  Part="1" 
AR Path="/5E264FF3/5E5940C4" Ref="PN109"  Part="1" 
AR Path="/5E2661DC/5E5940C4" Ref="PN136"  Part="1" 
AR Path="/5E26B65F/5E5940C4" Ref="PN190"  Part="1" 
AR Path="/5E26CFEA/5E5940C4" Ref="PN217"  Part="1" 
F 0 "PN217" H 4700 2750 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 4900 2750 50  0000 L CNN
F 2 "" H 4750 2650 50  0001 C CNN
F 3 "" H 4750 2650 50  0001 C CNN
F 4 "Molex" H 4850 2650 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 5100 2650 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 4750 2650 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 4750 2650 50  0001 C CNN "Supplier Part Number"
	1    4750 2650
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN147
U 1 1 5E5940D2
P 3750 3050
AR Path="/5E269D38/5E5940D2" Ref="PN147"  Part="1" 
AR Path="/5E24C641/5E5940D2" Ref="PN66"  Part="1" 
AR Path="/5E264FF3/5E5940D2" Ref="PN93"  Part="1" 
AR Path="/5E2661DC/5E5940D2" Ref="PN120"  Part="1" 
AR Path="/5E26B65F/5E5940D2" Ref="PN174"  Part="1" 
AR Path="/5E26CFEA/5E5940D2" Ref="PN201"  Part="1" 
F 0 "PN201" H 3700 3150 50  0000 L CNN
F 1 "Micro-Fit 20-24AWG" H 3900 3150 50  0000 L CNN
F 2 "" H 3750 3050 50  0001 C CNN
F 3 "" H 3750 3050 50  0001 C CNN
F 4 "Molex" H 3850 3050 50  0000 L CNN "Manufacturer"
F 5 "0430300002" H 4100 3050 50  0000 L CNN "Manufacturer part Number"
F 6 "Digi-Key" H 3750 3050 50  0001 C CNN "Supplier"
F 7 "WM1125CT-ND" H 3750 3050 50  0001 C CNN "Supplier Part Number"
	1    3750 3050
	1    0    0    -1  
$EndComp
$EndSCHEMATC
