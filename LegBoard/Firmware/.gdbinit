file Firmware.elf

define connect
    target remote | $OPENOCD/bin/openocd -f openocd.cfg

    # Info : stm32f7x.cpu: hardware has 8 breakpoints, 4 watchpoints
    set remote hardware-breakpoint-limit 8
    set remote hardware-watchpoint-limit 4
end

define reset
  monitor reset halt
end

define reload
  file Firmware.elf
  reset
  monitor stm32f2x mass_erase 0
  monitor program Firmware.elf verify
  reset
end

define setaddress
  tbreak prvIdleTask
  continue
  p modbus_parameters.address = $arg0
  continue
end

connect
# reload
# break main
# continue
