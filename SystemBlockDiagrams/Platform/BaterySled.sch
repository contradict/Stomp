EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 16 16
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
L Stomp:BMS10x0 BMS1
U 1 1 5E270B30
P 7950 2350
F 0 "BMS1" H 8175 3165 50  0000 C CNN
F 1 "BMS10x0" H 8175 3074 50  0000 C CNN
F 2 "" H 7950 2350 50  0001 C CNN
F 3 "https://www.roboteq.com/index.php/docman/bms-documents-and-files/bms-documents/datasheet/349-bms10x0-datasheet/file" H 7950 2350 50  0001 C CNN
F 4 "Roboteq" H 7950 2350 50  0001 C CNN "Supplier"
F 5 "BMS1040" H 7950 2350 50  0001 C CNN "Supplier Part Number"
	1    7950 2350
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_02x08_Odd_Even J133
U 1 1 5E27E551
P 6450 3500
F 0 "J133" H 6500 4017 50  0000 C CNN
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
L Connector_Generic:Conn_02x04_Top_Bottom J134
U 1 1 5E28024A
P 9400 3200
F 0 "J134" H 9450 3517 50  0000 C CNN
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
$Comp
L Stomp:LiPo6S2P BATT1
U 1 1 5E49A121
P 3700 3350
F 0 "BATT1" H 3282 3396 50  0000 R CNN
F 1 "LiPo6S2P" H 3282 3305 50  0000 R CNN
F 2 "" H 4150 3100 50  0001 C CNN
F 3 "" H 4150 3100 50  0001 C CNN
F 4 "Maxamps" H 3700 3350 50  0001 C CNN "Manufacturer"
F 5 "LiPo-8000-6S-22-2v" H 3700 3350 50  0001 C CNN "Manufacturer Part Number"
F 6 "Maxamps" H 3700 3350 50  0001 C CNN "Supplier"
F 7 "LiPo-8000-6S-22-2v" H 3700 3350 50  0001 C CNN "Supplier Part Number"
	1    3700 3350
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN223
U 1 1 5E4C6B8D
P 4900 3200
F 0 "PN223" H 4850 3350 50  0000 L CNN
F 1 "Female Pin" H 4850 3300 50  0000 L CNN
F 2 "" H 4900 3200 50  0001 C CNN
F 3 "" H 4900 3200 50  0001 C CNN
F 4 "Molex" H 5000 3200 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 3200 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 3200 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 3200 50  0001 C CNN "Supplier Part Number"
	1    4900 3200
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN224
U 1 1 5E4C7DF5
P 4900 3450
F 0 "PN224" H 4850 3600 50  0000 L CNN
F 1 "Female Pin" H 4850 3550 50  0000 L CNN
F 2 "" H 4900 3450 50  0001 C CNN
F 3 "" H 4900 3450 50  0001 C CNN
F 4 "Molex" H 5000 3450 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 3450 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 3450 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 3450 50  0001 C CNN "Supplier Part Number"
	1    4900 3450
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN225
U 1 1 5E4C7E03
P 4900 3700
F 0 "PN225" H 4850 3850 50  0000 L CNN
F 1 "Female Pin" H 4850 3800 50  0000 L CNN
F 2 "" H 4900 3700 50  0001 C CNN
F 3 "" H 4900 3700 50  0001 C CNN
F 4 "Molex" H 5000 3700 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 3700 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 3700 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 3700 50  0001 C CNN "Supplier Part Number"
	1    4900 3700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN226
U 1 1 5E4CB535
P 4900 3950
F 0 "PN226" H 4850 4100 50  0000 L CNN
F 1 "Female Pin" H 4850 4050 50  0000 L CNN
F 2 "" H 4900 3950 50  0001 C CNN
F 3 "" H 4900 3950 50  0001 C CNN
F 4 "Molex" H 5000 3950 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 3950 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 3950 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 3950 50  0001 C CNN "Supplier Part Number"
	1    4900 3950
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN227
U 1 1 5E4CB543
P 4900 4200
F 0 "PN227" H 4850 4350 50  0000 L CNN
F 1 "Female Pin" H 4850 4300 50  0000 L CNN
F 2 "" H 4900 4200 50  0001 C CNN
F 3 "" H 4900 4200 50  0001 C CNN
F 4 "Molex" H 5000 4200 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 4200 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 4200 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 4200 50  0001 C CNN "Supplier Part Number"
	1    4900 4200
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN228
U 1 1 5E4CB551
P 4900 4450
F 0 "PN228" H 4850 4600 50  0000 L CNN
F 1 "Female Pin" H 4850 4550 50  0000 L CNN
F 2 "" H 4900 4450 50  0001 C CNN
F 3 "" H 4900 4450 50  0001 C CNN
F 4 "Molex" H 5000 4450 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 4450 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 4450 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 4450 50  0001 C CNN "Supplier Part Number"
	1    4900 4450
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN229
U 1 1 5E4CB55F
P 4900 4700
F 0 "PN229" H 4850 4850 50  0000 L CNN
F 1 "Female Pin" H 4850 4800 50  0000 L CNN
F 2 "" H 4900 4700 50  0001 C CNN
F 3 "" H 4900 4700 50  0001 C CNN
F 4 "Molex" H 5000 4700 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 5250 4700 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 4900 4700 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 4900 4700 50  0001 C CNN "Supplier Part Number"
	1    4900 4700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN230
U 1 1 5E4D4522
P 9200 4200
F 0 "PN230" H 9150 4350 50  0000 L CNN
F 1 "Female Pin" H 9150 4300 50  0000 L CNN
F 2 "" H 9200 4200 50  0001 C CNN
F 3 "" H 9200 4200 50  0001 C CNN
F 4 "Molex" H 9300 4200 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 4200 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 4200 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 4200 50  0001 C CNN "Supplier Part Number"
	1    9200 4200
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN231
U 1 1 5E4D4530
P 9200 4450
F 0 "PN231" H 9150 4600 50  0000 L CNN
F 1 "Female Pin" H 9150 4550 50  0000 L CNN
F 2 "" H 9200 4450 50  0001 C CNN
F 3 "" H 9200 4450 50  0001 C CNN
F 4 "Molex" H 9300 4450 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 4450 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 4450 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 4450 50  0001 C CNN "Supplier Part Number"
	1    9200 4450
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN232
U 1 1 5E4D453E
P 9200 4700
F 0 "PN232" H 9150 4850 50  0000 L CNN
F 1 "Female Pin" H 9150 4800 50  0000 L CNN
F 2 "" H 9200 4700 50  0001 C CNN
F 3 "" H 9200 4700 50  0001 C CNN
F 4 "Molex" H 9300 4700 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 4700 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 4700 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 4700 50  0001 C CNN "Supplier Part Number"
	1    9200 4700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN233
U 1 1 5E4D454C
P 9200 4950
F 0 "PN233" H 9150 5100 50  0000 L CNN
F 1 "Female Pin" H 9150 5050 50  0000 L CNN
F 2 "" H 9200 4950 50  0001 C CNN
F 3 "" H 9200 4950 50  0001 C CNN
F 4 "Molex" H 9300 4950 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 4950 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 4950 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 4950 50  0001 C CNN "Supplier Part Number"
	1    9200 4950
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN234
U 1 1 5E4D455A
P 9200 5200
F 0 "PN234" H 9150 5350 50  0000 L CNN
F 1 "Female Pin" H 9150 5300 50  0000 L CNN
F 2 "" H 9200 5200 50  0001 C CNN
F 3 "" H 9200 5200 50  0001 C CNN
F 4 "Molex" H 9300 5200 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 5200 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 5200 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 5200 50  0001 C CNN "Supplier Part Number"
	1    9200 5200
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN235
U 1 1 5E4D4568
P 9200 5450
F 0 "PN235" H 9150 5600 50  0000 L CNN
F 1 "Female Pin" H 9150 5550 50  0000 L CNN
F 2 "" H 9200 5450 50  0001 C CNN
F 3 "" H 9200 5450 50  0001 C CNN
F 4 "Molex" H 9300 5450 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 5450 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 5450 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 5450 50  0001 C CNN "Supplier Part Number"
	1    9200 5450
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN236
U 1 1 5E4D4576
P 9200 5700
F 0 "PN236" H 9150 5850 50  0000 L CNN
F 1 "Female Pin" H 9150 5800 50  0000 L CNN
F 2 "" H 9200 5700 50  0001 C CNN
F 3 "" H 9200 5700 50  0001 C CNN
F 4 "Molex" H 9300 5700 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 5700 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 5700 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 5700 50  0001 C CNN "Supplier Part Number"
	1    9200 5700
	1    0    0    -1  
$EndComp
$Comp
L Stomp:PIN PN237
U 1 1 5E4D4584
P 9200 5950
F 0 "PN237" H 9150 6100 50  0000 L CNN
F 1 "Female Pin" H 9150 6050 50  0000 L CNN
F 2 "" H 9200 5950 50  0001 C CNN
F 3 "" H 9200 5950 50  0001 C CNN
F 4 "Molex" H 9300 5950 50  0000 L CNN "Manufacturer"
F 5 "0039000046" H 9550 5950 50  0000 L CNN "Manufacturer Part Number"
F 6 "Digi-Key" H 9200 5950 50  0001 C CNN "Supplier"
F 7 "WM2503-CT-ND" H 9200 5950 50  0001 C CNN "Supplier Part Number"
	1    9200 5950
	1    0    0    -1  
$EndComp
$EndSCHEMATC
