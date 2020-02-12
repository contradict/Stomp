file Firmware.elf

target remote | $OPENOCD/bin/openocd -f openocd.cfg

# Info : stm32f7x.cpu: hardware has 8 breakpoints, 4 watchpoints
set remote hardware-breakpoint-limit 8
set remote hardware-watchpoint-limit 4

define restart
  monitor reset halt
end

define reload
  restart
  monitor stm32f2x mass_erase 0
  monitor program Firmware.elf verify
  restart
end

# reload
# break main
# continue
