## Click1 - IMU / Telemetry Radio
  * Telemetry UART uart3 ttyS2
     * RXD  P9.22
     * TXD  P9.21
  * IMU SPI spi3
    * MISO P9.29
    * MOSI P9.30
    * SCK  P9.31
    * CS   P9.28 spi3_cs0
  * IMU I2C i2c4
    * SCL  P9.19
    * SDA  P9.20

## Click2 - ADC
  * ADC SPI spi3
    * MISO P9.29
    * MOSI P9.30
    * SCK  P9.31
    * CS   P9.42 spi3_cs1

## Click3 - Control Radio
  * Control UART uart10 ttyS1
    * RXD  P9.26
    * TXD  P9.24

## Click4 - RS485
  * RS485 UART uart5 ttyS4
    * RXD  P9.11
    * TXD  P9.13
    * DE   P8.06
