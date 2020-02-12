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
L Stomp:BMS10x0 BMS?
U 1 1 5E270B30
P 7950 2350
F 0 "BMS?" H 8175 3165 50  0000 C CNN
F 1 "BMS10x0" H 8175 3074 50  0000 C CNN
F 2 "" H 7950 2350 50  0001 C CNN
F 3 "https://www.roboteq.com/index.php/docman/bms-documents-and-files/bms-documents/datasheet/349-bms10x0-datasheet/file" H 7950 2350 50  0001 C CNN
F 4 "Roboteq" H 7950 2350 50  0001 C CNN "Supplier"
F 5 "BMS1040" H 7950 2350 50  0001 C CNN "Supplier Part Number"
	1    7950 2350
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5E2734E3
P 1750 2550
F 0 "BT?" H 1868 2646 50  0000 L CNN
F 1 "Battery_Cell" H 1868 2555 50  0000 L CNN
F 2 "" V 1750 2610 50  0001 C CNN
F 3 "~" V 1750 2610 50  0001 C CNN
	1    1750 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5E273940
P 1750 3050
F 0 "BT?" H 1868 3146 50  0000 L CNN
F 1 "Battery_Cell" H 1868 3055 50  0000 L CNN
F 2 "" V 1750 3110 50  0001 C CNN
F 3 "~" V 1750 3110 50  0001 C CNN
	1    1750 3050
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5E275C47
P 1750 3550
F 0 "BT?" H 1868 3646 50  0000 L CNN
F 1 "Battery_Cell" H 1868 3555 50  0000 L CNN
F 2 "" V 1750 3610 50  0001 C CNN
F 3 "~" V 1750 3610 50  0001 C CNN
	1    1750 3550
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5E275C4D
P 1750 4050
F 0 "BT?" H 1868 4146 50  0000 L CNN
F 1 "Battery_Cell" H 1868 4055 50  0000 L CNN
F 2 "" V 1750 4110 50  0001 C CNN
F 3 "~" V 1750 4110 50  0001 C CNN
	1    1750 4050
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5E276A37
P 1750 4550
F 0 "BT?" H 1868 4646 50  0000 L CNN
F 1 "Battery_Cell" H 1868 4555 50  0000 L CNN
F 2 "" V 1750 4610 50  0001 C CNN
F 3 "~" V 1750 4610 50  0001 C CNN
	1    1750 4550
	1    0    0    -1  
$EndComp
$Comp
L Device:Battery_Cell BT?
U 1 1 5E276A3D
P 1750 5050
F 0 "BT?" H 1868 5146 50  0000 L CNN
F 1 "Battery_Cell" H 1868 5055 50  0000 L CNN
F 2 "" V 1750 5110 50  0001 C CNN
F 3 "~" V 1750 5110 50  0001 C CNN
	1    1750 5050
	1    0    0    -1  
$EndComp
Text Notes 4550 1100 0    50   ~ 0
TODO:\n    Blind-make connector
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J?
U 1 1 5E27E551
P 6450 3500
F 0 "J?" H 6500 4017 50  0000 C CNN
F 1 "Conn_02x08_Odd_Even" H 6500 3926 50  0000 C CNN
F 2 "" H 6450 3500 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/039012160_sd.pdf" H 6450 3500 50  0001 C CNN
F 4 "Molex" H 6450 3500 50  0001 C CNN "Manufacturer"
F 5 "0039012160" H 6450 3500 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 6450 3500 50  0001 C CNN "Supplier"
F 7 "WM3707-ND" H 6450 3500 50  0001 C CNN "Supplier Part Number"
	1    6450 3500
	1    0    0    -1  
$EndComp
Text Notes 6100 4350 0    50   ~ 0
Balance Pins for 22-28AWG\nMolex 0039000046\nDigi-Key WM2503CT-ND
$Comp
L Connector_Generic:Conn_02x04_Top_Bottom J?
U 1 1 5E28024A
P 9400 3200
F 0 "J?" H 9450 3517 50  0000 C CNN
F 1 "Conn_02x04_Top_Bottom" H 9450 3426 50  0000 C CNN
F 2 "" H 9400 3200 50  0001 C CNN
F 3 "https://www.molex.com/pdm_docs/sd/039012080_sd.pdf" H 9400 3200 50  0001 C CNN
F 4 "Molex" H 9400 3200 50  0001 C CNN "Manufacturer"
F 5 "0039012080" H 9400 3200 50  0001 C CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9400 3200 50  0001 C CNN "Supplier"
F 7 "WM3703-ND" H 9400 3200 50  0001 C CNN "Supplier Part Number"
	1    9400 3200
	1    0    0    -1  
$EndComp
Text Notes 9050 3900 0    50   ~ 0
User Pins for 22-28AWG\nMolex 0039000046\nDigi-Key WM2503CT-ND
$EndSCHEMATC
