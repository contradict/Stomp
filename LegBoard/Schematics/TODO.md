* The rest of Chomp has pin 1 always ground. We should switch to this convention
* The 6-pin connector symbol and footprint have different pinouts. The footprint
    is correct.
* The Debug connector is difficult to install. It should be rotated 90 degrees
    or moved at least 0.5" from any tall components.
* Spread out the LEDs so each one is in line with the servo connectors it
    represents
* Add one more LED near the Communication connectors
* Add non-volatile storage. A >2kbit I2C EEPROM would be a good choice. It would
    also be interesting to explore the battery-backed SRAM on chip, maybe just a
    small lithium battery could solve this.
* I would like to have some testpoints
    * 4 microprocessor pins brought to plated holes
    * 6 LED controller pins brought to plated holes
