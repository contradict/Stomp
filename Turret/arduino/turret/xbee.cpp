#include "Arduino.h"
#include "xbee.h"
#include "pins.h"
#include "DMASerial.h"

extern DMASerial& Xbee;

ISR(PCINT2_vect)
{
   Xbee.cts_interrupt();
}

void xbeeInit(){
#ifdef HARD_WIRED
  Xbee.begin(115200);
#else
  Xbee.begin(57600);
  pinMode(XBEE_CTS, INPUT);
  *digitalPinToPCMSK(XBEE_CTS) |= bit (digitalPinToPCMSKbit(XBEE_CTS));
  PCIFR  |= bit (digitalPinToPCICRbit(XBEE_CTS));
  PCICR  |= bit (digitalPinToPCICRbit(XBEE_CTS));
  Xbee.set_cts_pin(XBEE_CTS);
#endif
}
