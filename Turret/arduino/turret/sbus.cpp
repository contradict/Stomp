#include <Arduino.h>
#include <util/atomic.h>

#include "pins.h"
#include "wiring_private.h"
#include "telemetryController.h"
#include "sbus.h"

static uint16_t computeRCBitfield();
static bool parseSbus();

static uint32_t last_sbus_time = 0;
static uint32_t radio_lost_timeout = 100000;
static uint8_t sbus_idx;
static uint8_t sbusData[25];
static bool new_packet = false;
uint16_t sbus_overrun;
static uint16_t sbusChannels [17];  // could initialize this with failsafe values for extra safety
static bool failsafe = true;
static uint32_t last_parse_time = 0;
static uint16_t last_bitfield;

volatile bool sbus_weaponsEnabled;
volatile static uint16_t bitfield;
volatile static bool s_radioConnected = false;

ISR(USART3_RX_vect)
{
    uint8_t c = UDR3;
    uint32_t now = micros();
    int32_t dt = now-last_sbus_time;
    last_sbus_time = now;
    if(dt>1000) {
        sbus_idx = 0;
    }
    if(!new_packet && (sbus_idx>0 || c=='\x0f')) {
        sbusData[sbus_idx++] = c;
    }
    if(sbus_idx == 25) {
        if(sbusData[0] == '\x0f' && sbusData[24] == 0) {
            new_packet = true;
        } else {
            sbus_overrun++;

        }
        sbus_idx = 0;
    }
}


ISR(USART3_UDRE_vect)
{
}

static void processSBus(void) {
    if(new_packet) {
        bool fail = parseSbus();
        if(!fail) {
            computeRCBitfield();
        } else {
            bitfield = 0;
            // BB MJS: Imediatly disable weapon on failed processSBus -> setWeaponsEnabled(false);
        }
        last_parse_time = micros();
        new_packet = false;
    }
}

void initSBus() {
    // Try u2x mode first
    uint32_t baud = 100000;
    uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
    UCSR3A = 1 << U2X0;

    // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
    UBRR3H = baud_setting >> 8;
    UBRR3L = baud_setting;

    UCSR3C = SERIAL_8N1;

    sbi(UCSR3B, RXEN0);
    sbi(UCSR3B, RXCIE0);
    last_parse_time = micros();
}

void updateSBus() 
{
    processSBus();
 
    bool timeout = ((micros() - last_parse_time) > radio_lost_timeout);

    s_radioConnected = !(failsafe || timeout);
    sbus_weaponsEnabled = (bitfield & WEAPONS_ENABLE_BIT) == WEAPONS_ENABLE_BIT;
}

bool isRadioConnected()
{
    return s_radioConnected;
}

bool isWeaponEnabled()
{
    return s_radioConnected && (bitfield & WEAPONS_ENABLE_BIT);
}

bool isManualTurretEnabled()
{
    return s_radioConnected && ((bitfield & MANUAL_TURRET_BIT) || (bitfield & AUTO_AIM_ENABLED_BIT));
}

bool isAutoAimEnabled()
{
    return s_radioConnected && (bitfield & AUTO_AIM_ENABLED_BIT);
}

bool isAutoFireEnabled()
{
    return s_radioConnected && (bitfield & AUTO_FIRE_ENABLE_BIT);
}

bool isSelfRightEnabled()
{
    return s_radioConnected && (bitfield & AUTO_SELF_RIGHT_BIT);
}

bool isFlameRightOnEnabled()
{
    return s_radioConnected && (bitfield & FLAME_RIGHT_CTRL_BIT);
}

bool isFlameRightPulseEnabled()
{
    return s_radioConnected && (bitfield & FLAME_RIGHT_PULSE_BIT);
}

bool hammerManualThrowAndRetract()
{
    return isWeaponEnabled() && (bitfield & HAMMER_FIRE_BIT);
}

bool hammerManualRetractOnly()
{
    return isWeaponEnabled() && (bitfield & HAMMER_RETRACT_BIT);
}

static bool parseSbus(){
    if (sbusData[0] == 0x0F && sbusData[24] == 0x00) {
        // perverse little endian-ish packet structure-- low bits come in first byte, remaining high bits
        // in next byte
        // NB: chars are promoted to shorts implicitly before bit shift operations
        sbusChannels[0]  = (sbusData[2]  << 8  | sbusData[1])                           & 0x07FF; // 8, 3
        sbusChannels[1]  = (sbusData[3]  << 5  | sbusData[2] >> 3)                      & 0x07FF; // 6, 5
        sbusChannels[2]  = (sbusData[5]  << 10 | sbusData[4] << 2 | sbusData[3] >> 6)   & 0x07FF; // 1, 8, 2
        sbusChannels[3]  = (sbusData[6]  << 7  | sbusData[5] >> 1)                      & 0x07FF; // 4, 7
        sbusChannels[4]  = (sbusData[7]  << 4  | sbusData[6] >> 4)                      & 0x07FF; // 7, 4
        sbusChannels[5]  = (sbusData[9]  << 9  | sbusData[8] << 1 | sbusData[7] >> 7)   & 0x07FF; // 2, 8, 1
        sbusChannels[6]  = (sbusData[10] << 6  | sbusData[9] >> 2)                      & 0x07FF; // 5, 6
        sbusChannels[7]  = (sbusData[11] << 3  | sbusData[10] >> 5)                     & 0x07FF; // 8, 3
        sbusChannels[8]  = (sbusData[13] << 8  | sbusData[12])                          & 0x07FF; // 3, 8
        sbusChannels[9]  = (sbusData[14] << 5  | sbusData[13] >> 3)                     & 0x07FF; // 6, 5
        sbusChannels[10] = (sbusData[16] << 10 | sbusData[15] << 2 | sbusData[14] >> 6) & 0x07FF; // 1, 8, 2
        sbusChannels[11] = (sbusData[17] << 7  | sbusData[16] >> 1)                     & 0x07FF; // 4, 7
        sbusChannels[12] = (sbusData[18] << 4  | sbusData[17] >> 4)                     & 0x07FF; // 7, 4
        sbusChannels[13] = (sbusData[20] << 9  | sbusData[19] << 1 | sbusData[18] >> 7) & 0x07FF; // 2, 8, 1
        sbusChannels[14] = (sbusData[21] << 6  | sbusData[20] >> 2)                     & 0x07FF; // 5, 6
        sbusChannels[15] = (sbusData[22] << 3  | sbusData[21] >> 5)                     & 0x07FF; // 8, 3
        failsafe = sbusData[23] & 0x08;
    }
    return failsafe;
}

#define WEAPONS_ENABLE_THRESHOLD 1450
#define AUTO_HAMMER_THRESHOLD 1000 // (190 down - 1800 up)
#define HAMMER_FIRE_THRESHOLD 1500 // 900 neutral, 170 to 1800
#define HAMMER_RETRACT_THRESHOLD 500

#define FLAME_PULSE_THRESHOLD 500
#define FLAME_CTRL_THRESHOLD 1500

#define MANUAL_TURRET_THRESHOLD 500
#define AUTO_AIM_THRESHOLD 1500

#define GENTLE_HAM_F_THRESHOLD 500
#define GENTLE_HAM_R_THRESHOLD 1500

#define TURRET_SPEED_RC_MIN 170
#define TURRET_SPEED_RC_MAX 1800
#define TURRET_SPEED_ROBOTECT_MIN -1000
#define TURRET_SPEED_ROBOTECT_MAX 1000
#define TURRET_SPEED_DEADZONE_MIN -10
#define TURRET_SPEED_DEADZONE_MAX 10

#define AUTO_SELF_RIGHT_THRESHOLD 1500
#define MANUAL_SELF_RIGHT_LEFT_THRESHOLD 500
#define MANUAL_SELF_RIGHT_RIGHT_THRESHOLD 1500

#define DANGER_MODE_THRESHOLD 1500

static uint16_t computeRCBitfield() {
  bitfield = 0;

  if(sbusChannels[WEAPONS_ENABLE] > WEAPONS_ENABLE_THRESHOLD) {
    bitfield |= WEAPONS_ENABLE_BIT;
  }

  if ( sbusChannels[AUTO_HAMMER_ENABLE] > AUTO_HAMMER_THRESHOLD){
    bitfield |= AUTO_FIRE_ENABLE_BIT;
  }

  if ( sbusChannels[HAMMER_CTRL] > HAMMER_FIRE_THRESHOLD){
    bitfield |= HAMMER_FIRE_BIT;
  }
  if ( sbusChannels[HAMMER_CTRL] < HAMMER_RETRACT_THRESHOLD){
    bitfield |= HAMMER_RETRACT_BIT;
  }

  // Full stick, flamethrower is on
  if (sbusChannels[FLAME_RIGHT_CTRL] > FLAME_CTRL_THRESHOLD){
    bitfield |= FLAME_RIGHT_CTRL_BIT;
  }
  // Center position enables pulse mode
  if (sbusChannels[FLAME_RIGHT_CTRL] > FLAME_PULSE_THRESHOLD && sbusChannels[FLAME_RIGHT_CTRL] < FLAME_CTRL_THRESHOLD ){
    bitfield |= FLAME_RIGHT_PULSE_BIT;
  }

 if( sbusChannels[TURRET_CTL_MODE] > AUTO_AIM_THRESHOLD ){
      bitfield |= AUTO_AIM_ENABLED_BIT;
  }
  else if(MANUAL_TURRET_THRESHOLD < sbusChannels[TURRET_CTL_MODE] &&
      sbusChannels[TURRET_CTL_MODE] < AUTO_AIM_THRESHOLD ){
      bitfield |= MANUAL_TURRET_BIT;
  }

  if ( sbusChannels[AUTO_SELF_RIGHT] > AUTO_SELF_RIGHT_THRESHOLD){
    bitfield |= AUTO_SELF_RIGHT_BIT;
  }

  // BB MJS: REmove
  // if ( sbusChannels[DANGER_MODE] > DANGER_MODE_THRESHOLD){
  //   bitfield |= DANGER_CTRL_BIT;
  // }

  return bitfield;
}

uint16_t getRcBitfield() {
    uint16_t bits;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        bits = bitfield;
    }
    return bits;
}

uint16_t getRcBitfieldChanges() {
    uint16_t changes;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        changes = bitfield ^ last_bitfield;
        last_bitfield = bitfield;
    }
    return changes;
}

// WARNING - this function assumes that you have successfully received an SBUS packet!
int16_t getSwingFillAngle()
{
  uint16_t channel_val;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      channel_val = sbusChannels[THROW_INTENSITY];
  }

  channel_val = constrain(channel_val, 172, 1811);
  int16_t angle = 52 + (channel_val - 172) * 5 / 11;

  return angle;
}

int16_t getDesiredManualTurretSpeed()
{
    uint16_t channel_val;
 
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        channel_val = sbusChannels[TURRET_SPIN];
    }

    //  Clamp the channel_val to the expected min max that we sould get (basically between 170 & 1800)
    //  Then map that to a turret speed, which is the value to write to the Robotec motor contoller (between -1000 and 1000)

    int16_t speed = constrain(channel_val, TURRET_SPEED_RC_MIN, TURRET_SPEED_RC_MAX);
    speed = map(speed, TURRET_SPEED_RC_MIN, TURRET_SPEED_RC_MAX, TURRET_SPEED_ROBOTECT_MAX, TURRET_SPEED_ROBOTECT_MIN);
    
    //  Have a dead zone in the middle
    if (speed > TURRET_SPEED_DEADZONE_MIN && speed < TURRET_SPEED_DEADZONE_MAX)
    {
        speed = 0;
    }

    return speed;
}

// 300-908, 600 mm neutral
uint16_t getRange() {
  uint16_t channel_val;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      channel_val = sbusChannels[RANGE];
  }
  if (channel_val < 172) { channel_val = 172; } else if (channel_val > 1811) { channel_val = 1811; }
  uint16_t range = (channel_val - 172) * 15 / 28 + 300;
  return range;
}
