
  * The fan connector was installed backwards, took it out and soldered a new one
in.

  * `5V` was not connected from our cape to the BeagleBone, I added a wire from
    `VDD_5V` (pins `P9.5` and `P9.6`) to the cape `5V` rail.

  * The `COMMUP/DOWN` line from the leg boards was connected to `P9.09`, which
    turns out to be the power button. I disconnected that for now.

  * `uart6` is connected to the WiFi/Bluetooth module on board, using it
    requires disabling WiFi. It would be nice to use a different UART for the
    control radio.
