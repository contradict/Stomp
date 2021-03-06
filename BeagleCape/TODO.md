
  * The fan connector was installed backwards, took it out and soldered a new one
in.

  * `5V` was not connected from our cape to the BeagleBone, I added a wire from
    `VDD_5V` (pins `P9.5` and `P9.6`) to the cape `5V` rail.

  * The `COMMUP/DOWN` line from the leg boards was connected to `P9.09`, which
    turns out to be the power button. I disconnected that for now.
    
  * Resistors:  If possible, no resistor smaller than 0603, any user installable
     jump resistor should be at least 0804, preferably 1206 if possible.
     
  * PWM?:  Put as many of the FET control inputs on header pins that can be
      connected to the PRU IO pins.  This might allow us to do clever peak
      and hold schemes with the big solenoids, or use the battery to run the
      ignitors with a 50% duty cycle.

  * Check all LED direction to make sure they are installed correctly (per Russel in slack on 10/31/20 the high current outputs appear to be soldered on backwards).
