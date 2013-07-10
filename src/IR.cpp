#include "stdafx.h"
#include "IR.h"

#include "LCD.h"

enum {
  STATE_IDLE = 0x0,
  STATE_START = 0x1,

  STATE_CMD_MASK = 0x20, // 6th bit
  STATE_DATA_MASK = 0x10, // 5th bit
  STATE_BIT_MASK = 0x0F, // bits counter mask (0..15), 0..7 - data byte, 8..15 - inverted byte
  STATE_BIT_INV = 0x08, // 8th bit = begin of inverted byte

  STATE_CMD_ADR = STATE_CMD_MASK, // 0x20
  STATE_CMD_ADR_INV = STATE_CMD_ADR | STATE_BIT_INV, // 0x28
  STATE_CMD_DATA = STATE_CMD_MASK | STATE_DATA_MASK, // 0x30
  STATE_CMD_DATA_INV = STATE_CMD_DATA | STATE_BIT_INV, // 0x38
  STATE_STOP = 0x40,

  STATE_ERROR = 0x80,
};

static volatile uint8_t state;
static volatile uint8_t data[2]; // Address, Command

void IR::init()
{
  // Configure 8-bit Counter0 for the time measurement
  TIMSK |= _BV(TOIE0); // enable Counter0 Overflow

  // Configure INT input
  MCUCR = _BV(ISC11) | _BV(ISC10); // generate INT1 on rising edge
  setbits(GICR, _BV(INT1)); // enable INT1

  state = STATE_IDLE;
}

//////////////////////////////////////////////////////////////////////////

static void startCounter()
{
  TCNT0 = 0x00; // init counter
  TCCR0 = _BV(CS01) | _BV(CS00); // set prescaler to 64
}

static void stopCounter()
{
  TCCR0 = 0x00; // Stop counter
}

// threshold = 10
// '0' = 18 ticks
// threshold = 20
// '1' = 35 ticks
// threshold = 40
// Repeat = 43 ticks
// threshold = 60
// Start = 78 ticks

#define TCNT_LOW 10
#define TCNT_ZERO 20
#define TCNT_ONE 40
#define TCNT_REPEAT 40
#define TCNT_START 60

ISR(INT1_vect)
{
  cli();
  uint8_t cnt = TCNT0;
  if(state == STATE_IDLE)
  {
    // First pulse, begin capturing
    startCounter();
    state = STATE_START;
  }
  else if(state == STATE_START)
  {
    if(cnt > TCNT_START)
    {
      // BEGIN
      startCounter();
      state = STATE_CMD_ADR;
    }
    else if(cnt > TCNT_REPEAT)
    {
      // REPEAT
      stopCounter();
      state = STATE_IDLE;

      // TODO: call handler again
      /*
      LCD::print('R');
      LCD::printDigit3(cnt);
      */
    }
    else
    {
      // Signal too short
      stopCounter();
      state = STATE_ERROR;
      /*
      LCD::print('!');
      LCD::printDigit3(cnt);
      */
    }
  }
  else if(state & STATE_CMD_MASK)
  {
    startCounter();

    uint8_t bit = (state & STATE_BIT_MASK); // bit = 0..15
    uint8_t dataIdx = (state & STATE_DATA_MASK) >> 4; // 0 - if state in STATE_CMD_ADR, 1 - if state in STATE_CMD_DATA

    if(cnt < TCNT_ZERO)
    {
      // '0'
      if(bit < STATE_BIT_INV)
      {
        // Set data bit by 0
        clrbits(data[dataIdx], 1 << bit);
      }
      else
      {
        // TODO: check inverted bit
        // getbits(data[dataIdx]), 1 << (bit - 8)) != 0
      }
      ++state; // move to the next bit
    }
    else if(cnt < TCNT_ONE)
    {
      // '1'
      if(bit < STATE_BIT_INV)
      {
        // Set data bit by 1
        setbits(data[dataIdx], 1 << bit);
      }
      else
      {
        // TODO: check inverted bit
        // getbits(data[dataIdx]), 1 << (bit - 8)) == 0
      }
      ++state; // move to the next bit
    }
    else
    {
      // Signal too long
      stopCounter();
      state = STATE_ERROR;
      /*
      LCD::print('?');
      LCD::printDigit3(cnt);
      */
    }

    if(state == STATE_STOP)
    {
      // Command received
      stopCounter();
      state = STATE_IDLE;

      // TODO: use command data
      LCD::cursorTo(1,0);
      LCD::printIn("ADR=");
      LCD::printDigit2(data[0], LCD::HEX);
      LCD::cursorTo(2,0);
      LCD::printIn("CMD=");
      LCD::printDigit2(data[1], LCD::HEX);
    }
  }

  sei();
}

ISR(TIMER0_OVF_vect)
{
  cli();
  // Failed to catch the sequence
  stopCounter();
  /*if(state != STATE_IDLE)
  {
    LCD::print('L');
  }*/
  state = STATE_IDLE;
  sei();
}