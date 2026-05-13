#ifndef IR_H
#define	IR_H

/*
 * Based on IRremote. 
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 * Edited by Mitra to add new controller SANYO
 * Ported to PIC18F2550 by Marco Koehler, 2013
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 * 
 * Just does receive with only hashing, no dcecoding
 */

#define RAWBUF 100 // Length of raw duration buffer

// Results returned from the decoder
typedef struct {
  int decode_type; // NEC, SONY, RC5, UNKNOWN
  unsigned int panasonicAddress; // This is only used for decoding Panasonic data
  unsigned long value; // Decoded value
  int bits; // Number of bits in decoded value
  volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
  unsigned int rawlen; // Number of records in rawbuf.
} decode_results;

// Values for decode_type
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define DISH 5
#define SHARP 6
#define PANASONIC 7
#define JVC 8
#define SANYO 9
#define MITSUBISHI 10
#define SIGMA 11
#define UNKNOWN -1

//Bit length of the protocolls
#define NEC_BITS 32
#define SIGMA_BITS 16
#define SONY_BITS 12
#define SANYO_BITS 12
#define MITSUBISHI_BITS 16
#define MIN_RC5_SAMPLES 11
#define MIN_RC6_SAMPLES 1
#define PANASONIC_BITS 48
#define PANASONIC_BITS_ADR 16
#define PANASONIC_BITS_VAL 32
#define JVC_BITS 16

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// return value of ir_decode()
#define ERR 0
#define DECODED 1

// call this function inside your InterruptServiceHigh()
extern void ir_interruptService(void);

// API calls
extern void ir_blink13(int blinkflag);
extern uint8_t ir_decode(decode_results *results);
extern void ir_enableIRIn(void);
extern void ir_resume(void);

extern void ir_delay(unsigned long time);


// receiver states
#define STATE_IDLE     2
#define STATE_MARK     3
#define STATE_SPACE    4
#define STATE_STOP     5

// IR detector output is active low
#define MARK  0
#define SPACE 1
#define LOW 0
#define OUTPUT 0
#define INPUT 1

#define TOPBIT 0x80000000

// information for the interrupt handler
typedef struct {
    unsigned char rcvstate;          // state machine
    unsigned int timer;     // state timer, counts 50uS ticks.
    unsigned int rawbuf[RAWBUF]; // raw data
    unsigned int rawlen;         // counter of entries in rawbuf
} irparams_t;

// Defined in IRremote.c
extern volatile irparams_t irparams;

#define DISABLE_INTERRUPTS   (INTCONbits.GIEH = 0)
#define ENABLE_INTERRUPTS    (INTCONbits.GIEH = 1)

// cpu speed
#define SYSCLOCK 16000000    // TCY - instructions per second of pic
#define USECPERTICK 50       // microseconds per clock interrupt tick

// defines for timers
#define MAX_TMR_VAL          65535
#define US_PER_SEC           1000000
#define TIMER_ENABLE_PWM     (CCPR1L=half_pwm)
#define TIMER_DISABLE_PWM    (CCPR1L=0)
#define TIMER_ENABLE_INTR    (PIE2bits.TMR3IE=1)   
#define TIMER_DISABLE_INTR   (PIE2bits.TMR3IE=0)
#define TIMER_INT_FLAG       PIR2bits.TMR3IF
#define TIMER_PWM_PIN        13
#define DELAY_INT_FLAG       PIR1bits.TMR1IF
#define DELAY_PRESCALE       4
#define DELAY_TICKS_PER_US   (SYSCLOCK/US_PER_SEC/DELAY_PRESCALE)

#define TOLERANCE 25  // percent tolerance in measurements
#define LTOL (1.0 - TOLERANCE/100.) 
#define UTOL (1.0 + TOLERANCE/100.) 

#define _GAP 5000 // Minimum map between transmissions
#define GAP_TICKS (_GAP/USECPERTICK)

#define TICKS_LOW(us) (int) (((us)*LTOL/USECPERTICK))
#define TICKS_HIGH(us) (int) (((us)*UTOL/USECPERTICK + 1))

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif	/* IR_H */

