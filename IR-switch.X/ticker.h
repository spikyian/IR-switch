#ifndef _TICKER_H_
/**
 * @file
 * @brief
 * This file provides access to all of the time management functions.
 * 
 * @details
 * Uses a hardware timer to implement timing functions.
 *
 * Uses 16bit PIC Timer0. Extends this to 32bit using timerExtension which is 
 * incremented on timer0 overflow.
 * Times are stored as TickValue.
 * 
 */
#define _TICKER_H_
/////////////////////////////////////////////////////
// TickTime
/////////////////////////////////////////////////////
    /* this section is based on the Timer 0 module of the PIC18 family */

//   Prescaler is now calculated from clock MHz in the init routine - which assumes clock is an exact number of MHz
//    #define ONE_SECOND (((DWORD)CLOCK_FREQ/1000 * 62500) / (SYMBOL_TO_TICK_RATE / 1000))
//    /* SYMBOLS_TO_TICKS to only be used with input (a) as a constant, otherwise you will blow up the code */
//    #define SYMBOLS_TO_TICKS(a) (((DWORD)CLOCK_FREQ/100000) * a / ((DWORD)SYMBOL_TO_TICK_RATE/100000))
//    #define TICKS_TO_SYMBOLS(a) (((DWORD)SYMBOL_TO_TICK_RATE/100000) * a / ((DWORD)CLOCK_FREQ/100000))


#define TMR_IF          INTCONbits.TMR0IF
#define TMR_IE          INTCONbits.TMR0IE
#define TMR_IP          INTCON2bits.TMR0IP
#define TMR_ON          T0CONbits.TMR0ON
#define TMR_MODE        T0CONbits.T08BIT
#define TMR_PS          T0CON
#define TMR_CS          T0CONbits.T0CS
#define TMR_L           TMR0L
#define TMR_H           TMR0H


/*
 * TimeVal counter values.
 * 6 ticks is 96us - approx 100us as close as possible with 16uS resolution.
 * Equates to 62500 ticks per second.
 */
#define HUNDRED_MICRO_SECOND 6                      ///< TimeVal value for 0.0001 seconds.
#define ONE_SECOND          62500                   ///< TimeVal value for 1 second.
#define TWO_SECOND          (ONE_SECOND*2)          ///< TimeVal value for 2 seconds.
#define FIVE_SECOND         (ONE_SECOND*5)          ///< TimeVal value for 5 seconds.
#define TEN_SECOND          (ONE_SECOND*10)         ///< TimeVal value for 10 seconds.
#define HALF_SECOND         (ONE_SECOND/2)          ///< TimeVal value for 0.5 seconds.
#define HALF_MILLI_SECOND   (ONE_SECOND/2000)       ///< TimeVal value for 0.0005 seconds.
#define ONE_MILI_SECOND     (ONE_SECOND/1000)       ///< TimeVal value for 0.001 seconds.
#define HUNDRED_MILI_SECOND (ONE_SECOND/10)         ///< TimeVal value for 0.1 seconds.
#define FORTY_MILI_SECOND   (ONE_SECOND/25)         ///< TimeVal value for 0.04 seconds.
#define TWENTY_MILI_SECOND  (ONE_SECOND/50)         ///< TimeVal value for 0.02 seconds.
#define TEN_MILI_SECOND     (ONE_SECOND/100)        ///< TimeVal value for 0.01 seconds.
#define FIVE_MILI_SECOND    (ONE_SECOND/200)        ///< TimeVal value for 0.005 seconds.
#define TWO_MILI_SECOND     (ONE_SECOND/500)        ///< TimeVal value for 0.002 seconds.
#define ONE_MINUTE          (ONE_SECOND*60)         ///< TimeVal value for 1 minute.
#define ONE_HOUR            (ONE_MINUTE*60)         ///< TimeVal value for 1 hour.

/**
 * Calculate the time between two TimeVal. Note that result is signed.
 */
#define tickGetDiff(a,b) (a.val - b.val)
/**
 * Calculate the time from the given TimeVal to now.
 */
#define tickTimeSince(t)    (tickGet() - t.val)

/************************ DATA TYPES *******************************/


/**
 * Time unit defined based on IEEE 802.15.4 specification.
 * One tick is equal to one symbol time, or 16us. The Tick structure
 * is four bytes in length and is capable of represent time up to
 * about 19 hours.
 */
typedef union _TickValue {
    /** Provides access to the value as a 32bit.*/
    uint32_t val;    ///< TickValue as a 32bit value.
    /** Provides access to the value as bytes.*/
    struct TickBytes    
    {
        uint8_t b0; ///< The first byte of the TickValue.
        uint8_t b1; ///< The second byte of the TickValue.
        uint8_t b2; ///< The third byte of the TickValue.
        uint8_t b3; ///< The forth byte of the TickValue.
    } byte;
    uint8_t v[4]; ///< TickValue as an array of bytes.
    /** Provides access to the value as two 16bits.*/
    struct TickWords    
    {
        uint16_t w0; ///< The first word of the TickValue.
        uint16_t w1; ///< The second word of the TickValue.
    } word;
} TickValue;


// Global routine definitions

/**
 * Sets up Timer0 to count time.
 * @param priority 0=low priority, high priority otherwise
 */
void initTicker(uint8_t priority);

/**
 * Gets the current tick counter indicating time since power on.
 * @return the value of the timer
 */
uint32_t tickGet(void);


/* *********************** VARIABLES ********************************/
/**
 * Timer0 provides a 16bit counter. The timerExtension variables extend 
 * the count to 32bit. timerExtension1 provides the lower extension byte.
 */
extern volatile uint8_t timerExtension1;
/**
 * Timer0 provides a 16bit counter. The timerExtension variables extend 
 * the count to 32bit. timerExtension2 provides the top most byte.
 */
extern volatile uint8_t timerExtension2;

/**
 ISR
 */
extern void tickerIsr(void);

#endif  // TICKER_H
