
/**
 * @file
 * @brief
 * Functions to provide timing.
 * 
 * @details
 * Uses 16bit PIC Timer0. Extends this to 32bit using timerExtension which is 
 * incremented on timer0 overflow.
 * Times are stored as TickValue.
 * Functions are provided to measure time since a time value was recorded.
 * 
 */

#include <xc.h>
#include "ticker.h"
#include "hardware.h"


/** 
 * Variable used to hold the upper bytes of the timeVal. 
 * Extends the hardware counter from 16bits to 32bits.
 */
volatile uint8_t timerExtension1,timerExtension2;

/************************ FUNCTIONS ********************************/

/**
 * Initialise the tick timer using the specified interrupt priority.
 * Function:         void InitTicker()
 *
 * TMR0 for PIC18 is configured for calculating the correct symbol times.  
 * The timer interrupt is enabled causing the timer roll over calculations.  
 * Interrupts are required to be enabled in order to extend the timer to
 * 4 bytes in PIC18.  PIC24/dsPIC version do not enable or require interrupts.
 */
void initTicker(uint8_t priority) {
    uint8_t divider, i;

    divider = 0;
    for (i=clkMHz;i>0;i>>=1) // Work out timer prescaler value from clock MHz
        divider++;

#if defined(_18F66K80_FAMILY_)
    TMR_PS = (uint8_t)(0b00000000 | divider);     // Enable clock prescaler on and set prescaler value
    TMR_MODE = 0;       // 16 bit mode
    TMR_CS = 0;         // Fosc clock source
    TMR_H = 0;          // clear the H buffer
    TMR_L = 0;          // write the L counter and load the H counter from buffer
    TMR_IP = priority;  // set interrupt priority
    TMR_IF = 0;         // clear the flag
    TMR_IE = 1;         // enable interrupts
    TMR_ON = 1;         // start it running

    timerExtension1 = 0;
    timerExtension2 = 0;
#elif defined(_18FXXQ83_FAMILY_)
    TMR_PS = (uint8_t)(0b00000000 | (divider+1));     // Enable internal clock, prescaler on and set prescaler value
    TMR_MODE = 1;       // 16 bit mode
    TMR_CS = 2;         // Fosc/4 clock source
    TMR_H = 0;          // clear the H buffer
    TMR_L = 0;          // write the L counter and load the H counter from buffer
    TMR_IP = (__bit)priority;  // set interrupt priority
    TMR_IF = 0;         // clear the flag
    TMR_IE = 1;         // enable interrupts
    TMR_ON = 1;         // start it running

    timerExtension1 = 0;
    timerExtension2 = 0;

#elif defined(__dsPIC30F__) || defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
    T2CON = 0b0000000000001000 | CLOCK_DIVIDER_SETTING;
    T2CONbits.TON = 1;
#elif defined(__PIC32MX__)
    CloseTimer2();
    WriteTimer2(0x00);
    WriteTimer3(0x00);
    WritePeriod3(0xFFFF);
    OpenTimer2((T2_ON|T2_32BIT_MODE_ON|CLOCK_DIVIDER_SETTING),0xFFFFFFFF);     
#else
    #error "Invalid Processor defines in ticktime.c"
#endif
}


/**
 * Return the current tick time.
 * PIC18 only: the timer interrupt is disabled for several instruction cycles 
 * while the timer value is grabbed.  This is to prevent a rollover from 
 * incrementing the timer extenders during the read of their values.
 *
 * @return the 32bit timer value
 */
uint32_t tickGet(void) {
    TickValue currentTime;
    
    //uint8_t failureCounter;
    uint8_t IntFlag1;
    uint8_t IntFlag2;
    
    /* zero the byte extension for now*/
    currentTime.byte.b2 = 0;
    currentTime.byte.b3 = 0;
    /* disable the timer interrupt to prevent roll over of the lower 16 bits while before/after reading of the extension */
    TMR_IE = 0;
    do {
        IntFlag1 = TMR_IF;
        currentTime.byte.b0 = TMR_L;
        currentTime.byte.b1 = TMR_H;    // PIC latched the H register whist reading the L register. Safe 2 byte read.
        IntFlag2 = TMR_IF;
    } while(IntFlag1 != IntFlag2);  // verify that a rollover didn't happen during getting the counter

    if( IntFlag1 > 0 ) {         // if a rollover did happen then handle it here instead of in ISR
        TMR_IF = 0;
        timerExtension1++;
        if(timerExtension1 == 0)
        {
            timerExtension2++;
        }
    }

    /* copy the byte extension */
    currentTime.byte.b2 += timerExtension1;
    currentTime.byte.b3 += timerExtension2;
    
    /* re-enable the timer interrupt */
    TMR_IE = 1;
    
    return currentTime.val;
} // tickGet

void tickerIsr(void) {
    // Tick Timer interrupt
    //check to see if the symbol timer overflowed
    if(TMR_IF) {
        /* there was a timer overflow */
        TMR_IF = 0;
        timerExtension1++;
        if(timerExtension1 == 0) {
            timerExtension2++;
        }
    }
    return;
}
