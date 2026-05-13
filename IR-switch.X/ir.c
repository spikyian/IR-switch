/*
 * File:   ir.c
 * Author: Ian
 *
 * Created on 12 May 2026, 09:19
 */

/*
 * Based on IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 * Ported to PIC18F2550 by Marco Koehler, 2013
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#include <xc.h>
#include "ir.h"
#include "hardware.h"

volatile irparams_t irparams;
static void ir_timerRst(void);
static void ir_timerCfgNorm(void);
static long ir_decodeHash(decode_results *results);
static uint8_t MATCH(unsigned int measured, int desired);
static uint8_t MATCH_MARK(unsigned int measured_ticks, int desired_us);
static uint8_t MATCH_SPACE(unsigned int measured_ticks, int desired_us);

// initialisation
void ir_enableIRIn(void) {
    BLINKLED_OFF();
    // initialise state machine variables
    irparams.rcvstate = STATE_IDLE;
    irparams.rawlen = 0;
    // set pin modes
    IR_RECEIVE_DIRECTION = INPUT;

    DISABLE_INTERRUPTS;
    // setup pulse clock timer interrupt for Timer
    ir_timerCfgNorm();
    ir_timerRst();

    //Timer2 Overflow Interrupt Enable
    TIMER_ENABLE_INTR;

    ENABLE_INTERRUPTS;  // enable interrupts
}

volatile unsigned char half_pwm = 0;


/**
 * Set up TIMER3 peripheral to measure the received pulse lengths.
 */
static void ir_timerCfgNorm(void) {
  /*timer 3 for ir-receiving*/
  IPR2bits.TMR3IP = 1; // high priority
  T3CON = 0b10000100;
  TMR3H = (MAX_TMR_VAL - (USECPERTICK*(SYSCLOCK/US_PER_SEC)))/256;
  TMR3L = (MAX_TMR_VAL - (USECPERTICK*(SYSCLOCK/US_PER_SEC)))%256;
  PIR2bits.TMR3IF = 0;
  IPR2bits.TMR3IP = 1;
  T3CONbits.TMR3ON = 1;
}

/**
 * Reset the timer ready to count another pulse.
 */
static void ir_timerRst(void) {
    /*timer 3 for ir-receiving*/
    TMR3H = (MAX_TMR_VAL - (USECPERTICK*(SYSCLOCK/US_PER_SEC)))/256;
    TMR3L = (MAX_TMR_VAL - (USECPERTICK*(SYSCLOCK/US_PER_SEC)))%256;
}

// TIMER interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts

// call this function inside your InterruptServiceHigh()
void ir_interruptService(void)
{
    unsigned char irdata = 0;

    // timer is used for sampling IR signal
    if (TIMER_INT_FLAG == 1)
    {
        TIMER_INT_FLAG = 0;

        ir_timerRst();

        irdata = (unsigned char)(IR_RECEIVE_PIN);

        irparams.timer++; // One more 50us tick
        if (irparams.rawlen >= RAWBUF) {
            // Buffer overflow
            irparams.rcvstate = STATE_STOP;
        }
        switch(irparams.rcvstate) {
          case STATE_IDLE: // In the middle of a gap
            if (irdata == MARK) {
                if (irparams.timer < GAP_TICKS) {
                    // Not big enough to be a gap.
                    irparams.timer = 0;
                } 
                else {
                    // gap just ended, record duration and start recording transmission
                    irparams.rawlen = 0;
                    irparams.rawbuf[irparams.rawlen++] = irparams.timer;
                    irparams.timer = 0;
                    irparams.rcvstate = STATE_MARK;
                }
            }
            break;
          case STATE_MARK: // timing MARK
            if (irdata == SPACE) {   // MARK ended, record time
                irparams.rawbuf[irparams.rawlen++] = irparams.timer;
                irparams.timer = 0;
                irparams.rcvstate = STATE_SPACE;
            }
            break;
          case STATE_SPACE: // timing SPACE
            if (irdata == MARK) { // SPACE just ended, record it
                irparams.rawbuf[irparams.rawlen++] = irparams.timer;
                irparams.timer = 0;
                irparams.rcvstate = STATE_MARK;
            } else { // SPACE
                if (irparams.timer > GAP_TICKS) {
                    // big SPACE, indicates gap between codes
                    // Mark current code as ready for processing
                    // Switch to STOP
                    // Don't reset timer; keep counting space width
                    irparams.rcvstate = STATE_STOP;
                } 
            }
            break;
         case STATE_STOP: // waiting, measuring gap
            if (irdata == MARK) { // reset gap timer
                irparams.timer = 0;
            }
            break;
        }

        if (irdata == MARK) {
            BLINKLED_ON(); 
        } 
        else {
            BLINKLED_OFF(); 
        }
    }
}

/**
 * Get ready to receive the next IR code.
 */
void ir_resume(void) {
    irparams.rcvstate = STATE_IDLE;
    irparams.rawlen = 0;
}


// Decodes the received IR message
// Returns ERR if no data ready, DECODED if data ready.
// Results of decoding are stored in results
uint8_t ir_decode(decode_results *results) {
  results->rawlen = irparams.rawlen;
  results->rawbuf = (volatile unsigned int *)&irparams.rawbuf[0];
  if (irparams.rcvstate != STATE_STOP) {
    return ERR;
  }

  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  if (ir_decodeHash(results)) {
    return DECODED;
  }
  // Throw away and start over
  ir_resume();
  return ERR;
}


// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
static int ir_getRClevel(decode_results *results, int *offset, int *used, int t1) {
  unsigned int width = 0;
  int val = 0;
  int correction = 0;
  int avail = 0;
  if (*offset >= results->rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  width = results->rawbuf[*offset];
  val = ((*offset) % 2) ? MARK : SPACE;
  correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  if (MATCH(width, t1 + correction)) {
    avail = 1;
  } 
  else if (MATCH(width, 2*t1 + correction)) {
    avail = 2;
  } 
  else if (MATCH(width, 3*t1 + correction)) {
    avail = 3;
  } 
  else {
    return -1;
  }

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
  return val;   
}


/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
static uint8_t ir_compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } 
  else if (oldval < newval * .8) {
    return 2;
  } 
  else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
static long ir_decodeHash(decode_results *results)
{
  unsigned long hash = FNV_BASIS_32;
  int i = 0;
  // Require at least 6 samples to prevent triggering on noise
  if (results->rawlen < 6) {
    return ERR;
  }

  for (i = 1; i+2 < results->rawlen; i++) {
    uint8_t value =  ir_compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value = hash;
  results->bits = 32;
  results->decode_type = UNKNOWN;
  return DECODED;
}


static uint8_t MATCH(unsigned int measured, int desired)
{
    return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

static uint8_t MATCH_MARK(unsigned int measured_ticks, int desired_us)
{
    return MATCH(measured_ticks, (desired_us + MARK_EXCESS));
}

static uint8_t MATCH_SPACE(unsigned int measured_ticks, int desired_us)
{
    return MATCH(measured_ticks, (desired_us - MARK_EXCESS));
}