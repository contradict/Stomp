EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 15
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
AR Path="/5E24C641/5E255C4B" Ref="SV?"  Part="1" 
AR Path="/5E264FF3/5E255C4B" Ref="SV?"  Part="1" 
AR Path="/5E2661DC/5E255C4B" Ref="SV?"  Part="1" 
AR Path="/5E269D38/5E255C4B" Ref="SV?"  Part="1" 
AR Path="/5E26B65F/5E255C4B" Ref="SV?"  Part="1" 
AR Path="/5E26CFEA/5E255C4B" Ref="SV?"  Part="1" 
F 0 "SV?" H 1917 2065 50  0000 C CNN
F 1 "Enfield S2" H 1917 1974 50  0000 C CNN
F 2 "" H 1150 1100 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 1150 1100 50  0001 C CNN
	1    1950 1400
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E255C55
P 6400 1500
AR Path="/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255C55" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255C55" Ref="J?"  Part="1" 
F 0 "J?" H 6450 1907 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6450 1816 50  0000 C CNN
F 2 "" H 6400 1500 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250600/WM1785-ND/252498" H 6400 1500 50  0001 C CNN
F 4 "Digi-Key" H 6400 1500 50  0001 C CNN "Supplier"
F 5 "WM1785-ND " H 6450 1725 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 6400 1500 50  0001 C CNN "Manufacturer"
F 7 "0430250600" H 6400 1500 50  0001 C CNN "Manufacturer Part Number"
	1    6400 1500
	1    0    0    -1  
$EndComp
Text Notes 5500 850  0    50   ~ 0
Microfit Female Pins 20-24AWG, Gold\nMolex 0430300002\nDigi-Key  WM1125CT-ND 
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E255C60
P 6350 2100
AR Path="/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255C60" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255C60" Ref="J?"  Part="1" 
F 0 "J?" H 6458 2381 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6458 2290 50  0000 C CNN
F 2 "" H 6350 2100 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 6350 2100 50  0001 C CNN
F 4 "Molex" H 6350 2100 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 6350 2100 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6350 2100 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 6450 1900 50  0000 C CNN "Supplier Part Number"
	1    6350 2100
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
P 5450 2100
AR Path="/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E24C641/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E264FF3/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E2661DC/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E269D38/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E26B65F/5E255C75" Ref="U?"  Part="1" 
AR Path="/5E26CFEA/5E255C75" Ref="U?"  Part="1" 
F 0 "U?" H 5417 2605 50  0000 C CNN
F 1 "AngleSensor" H 5417 2514 50  0000 C CNN
F 2 "" H 5000 1850 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5000 1850 50  0001 C CNN
F 4 "TT Electronics" H 5450 2100 50  0001 C CNN "Manufacturer"
F 5 "6127V1A60L.5" H 5417 2423 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 5450 2100 50  0001 C CNN "Supplier"
F 7 "858-6127V1A60L.5" H 5450 2100 50  0001 C CNN "Supplier Part Number"
	1    5450 2100
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E255C7B
P 1950 2850
AR Path="/5E255C7B" Ref="SV?"  Part="1" 
AR Path="/5E24C641/5E255C7B" Ref="SV?"  Part="1" 
AR Path="/5E264FF3/5E255C7B" Ref="SV?"  Part="1" 
AR Path="/5E2661DC/5E255C7B" Ref="SV?"  Part="1" 
AR Path="/5E269D38/5E255C7B" Ref="SV?"  Part="1" 
AR Path="/5E26B65F/5E255C7B" Ref="SV?"  Part="1" 
AR Path="/5E26CFEA/5E255C7B" Ref="SV?"  Part="1" 
F 0 "SV?" H 1917 3515 50  0000 C CNN
F 1 "Enfield S2" H 1917 3424 50  0000 C CNN
F 2 "" H 1150 2550 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 1150 2550 50  0001 C CNN
	1    1950 2850
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:PneumaticServoValve SV?
U 1 1 5E255C81
P 1950 4300
AR Path="/5E255C81" Ref="SV?"  Part="1" 
AR Path="/5E24C641/5E255C81" Ref="SV?"  Part="1" 
AR Path="/5E264FF3/5E255C81" Ref="SV?"  Part="1" 
AR Path="/5E2661DC/5E255C81" Ref="SV?"  Part="1" 
AR Path="/5E269D38/5E255C81" Ref="SV?"  Part="1" 
AR Path="/5E26B65F/5E255C81" Ref="SV?"  Part="1" 
AR Path="/5E26CFEA/5E255C81" Ref="SV?"  Part="1" 
F 0 "SV?" H 1917 4965 50  0000 C CNN
F 1 "Enfield S2" H 1917 4874 50  0000 C CNN
F 2 "" H 1150 4000 50  0001 C CNN
F 3 "https://github.com/contradict/Stomp/blob/master/Datasheets/S2_Datasheet.pdf" H 1150 4000 50  0001 C CNN
	1    1950 4300
	-1   0    0    -1  
$EndComp
$Comp
L Stomp:AngleSensor U?
U 1 1 5E255C8B
P 5500 3450
AR Path="/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E24C641/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E264FF3/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E2661DC/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E269D38/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E26B65F/5E255C8B" Ref="U?"  Part="1" 
AR Path="/5E26CFEA/5E255C8B" Ref="U?"  Part="1" 
F 0 "U?" H 5467 3955 50  0000 C CNN
F 1 "AngleSensor" H 5467 3864 50  0000 C CNN
F 2 "" H 5050 3200 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5050 3200 50  0001 C CNN
F 4 "TT Electronics" H 5500 3450 50  0001 C CNN "Manufacturer"
F 5 "6127V1A45L.5" H 5467 3773 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 5500 3450 50  0001 C CNN "Supplier"
F 7 "858-6127V1A45L.5" H 5500 3450 50  0001 C CNN "Supplier Part Number"
	1    5500 3450
	-1   0    0    -1  
$EndComp
Text Notes 1000 1450 0    197  ~ 0
Curl
Text Notes 1050 3000 0    197  ~ 0
Lift
Text Notes 1000 4300 0    197  ~ 0
Swing
Text Notes 4550 2100 0    50   ~ 0
60 Degree sensor
$Comp
L Stomp:AngleSensor U?
U 1 1 5E255C99
P 5500 4900
AR Path="/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E24C641/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E264FF3/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E2661DC/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E269D38/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E26B65F/5E255C99" Ref="U?"  Part="1" 
AR Path="/5E26CFEA/5E255C99" Ref="U?"  Part="1" 
F 0 "U?" H 5467 5405 50  0000 C CNN
F 1 "AngleSensor" H 5467 5314 50  0000 C CNN
F 2 "" H 5050 4650 50  0001 C CNN
F 3 "https://www.mouser.com/datasheet/2/414/6120-1548265.pdf" H 5050 4650 50  0001 C CNN
F 4 "TT Electronics" H 5500 4900 50  0001 C CNN "Manufacturer"
F 5 "6127V1A45L.5" H 5467 5223 50  0000 C CNN "Manufacturer Part Number"
F 6 "Mouser" H 5500 4900 50  0001 C CNN "Supplier"
F 7 "858-6127V1A45L.5" H 5500 4900 50  0001 C CNN "Supplier Part Number"
	1    5500 4900
	-1   0    0    -1  
$EndComp
Text Notes 4300 3000 0    50   ~ 0
Micro-Fit Female Pins 26-30AWG, Gold\nMolex 0430300005\nDigi-Key WM9169CT-ND
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E255CA4
P 6400 3450
AR Path="/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CA4" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CA4" Ref="J?"  Part="1" 
F 0 "J?" H 6508 3731 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6508 3640 50  0000 C CNN
F 2 "" H 6400 3450 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 6400 3450 50  0001 C CNN
F 4 "Molex" H 6400 3450 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 6400 3450 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6400 3450 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 6400 3450 50  0001 C CNN "Supplier Part Number"
	1    6400 3450
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x03_Male J?
U 1 1 5E255CAE
P 6400 4900
AR Path="/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CAE" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CAE" Ref="J?"  Part="1" 
F 0 "J?" H 6508 5181 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6508 5090 50  0000 C CNN
F 2 "" H 6400 4900 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0436400301/WM1856-ND/268985" H 6400 4900 50  0001 C CNN
F 4 "Molex" H 6400 4900 50  0001 C CNN "Manufcturer"
F 5 "0436400301" H 6400 4900 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6400 4900 50  0001 C CNN "Suplier"
F 7 " WM1856-ND " H 6400 4900 50  0001 C CNN "Supplier Part Number"
	1    6400 4900
	1    0    0    -1  
$EndComp
$Comp
L Connector:DIN-5 J?
U 1 1 5E255CB6
P 3050 1300
AR Path="/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CB6" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CB6" Ref="J?"  Part="1" 
F 0 "J?" H 3050 1025 50  0000 C CNN
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
AR Path="/5E24C641/5E255CBE" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CBE" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CBE" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CBE" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CBE" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CBE" Ref="J?"  Part="1" 
F 0 "J?" H 3050 2475 50  0000 C CNN
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
AR Path="/5E24C641/5E255CC6" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CC6" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CC6" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CC6" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CC6" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CC6" Ref="J?"  Part="1" 
F 0 "J?" H 3050 3925 50  0000 C CNN
F 1 "M8 5 Pin" H 3050 3834 50  0000 C CNN
F 2 "" H 3050 4200 50  0001 C CNN
F 3 "https://www.mcmaster.com/7138k39" H 3050 4200 50  0001 C CNN
F 4 "McMaster-Carr" H 3050 4200 50  0001 C CNN "Supplier"
F 5 "7138K39" H 3050 4200 50  0001 C CNN "Supplier Part Number"
	1    3050 4200
	1    0    0    -1  
$EndComp
Text Notes 2750 900  0    50   ~ 0
Comes with 6ft cable
Text Notes 4300 2700 0    50   ~ 0
Sensor cable is\n26AWG 3 conductor\nCooner Wire NMEF 3/26-6544 J
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E255CD2
P 6400 2850
AR Path="/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CD2" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CD2" Ref="J?"  Part="1" 
F 0 "J?" H 6450 3257 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6450 3166 50  0000 C CNN
F 2 "" H 6400 2850 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250600/WM1785-ND/252498" H 6400 2850 50  0001 C CNN
F 4 "Digi-Key" H 6400 2850 50  0001 C CNN "Supplier"
F 5 "WM1785-ND " H 6450 3075 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 6400 2850 50  0001 C CNN "Manufacturer"
F 7 "0430250600" H 6400 2850 50  0001 C CNN "Manufacturer Part Number"
	1    6400 2850
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x03_Odd_Even J?
U 1 1 5E255CDC
P 6450 4300
AR Path="/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E24C641/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E264FF3/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E2661DC/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E269D38/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E26B65F/5E255CDC" Ref="J?"  Part="1" 
AR Path="/5E26CFEA/5E255CDC" Ref="J?"  Part="1" 
F 0 "J?" H 6500 4707 50  0000 C CNN
F 1 "Micro-Fit 3.0 Plug" H 6500 4616 50  0000 C CNN
F 2 "" H 6450 4300 50  0001 C CNN
F 3 "https://www.digikey.com/product-detail/en/molex/0430250600/WM1785-ND/252498" H 6450 4300 50  0001 C CNN
F 4 "Digi-Key" H 6450 4300 50  0001 C CNN "Supplier"
F 5 "WM1785-ND " H 6500 4525 50  0000 C CNN "Supplier Part Number"
F 6 "Molex" H 6450 4300 50  0001 C CNN "Manufacturer"
F 7 "0430250600" H 6450 4300 50  0001 C CNN "Manufacturer Part Number"
	1    6450 4300
	1    0    0    -1  
$EndComp
$EndSCHEMATC
