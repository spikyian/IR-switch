/*
 * File:   IRswitchMain.c
 * Author: Ian
 *
 * Created on 11 May 2026, 12:07
 * 
 * Infra red switch. Allows any infra-red controller/transmitter to turn an output
 * on or off. Two buttons on the controller are used, one to turn the output on 
 * and the other to turn it off.
 * Two IR codes can be stored for the on and off buttons by putting the module
 * into its configuration mode. A switch is used to put the module into 
 * configuration mode.
 * 
 * The following LED indications are provided:
 * - A blink LED to indicate that an IR signal is being received.
 * - The module is in configuration mode and the ON button is being set.
 * - The module is in configuration mode and the OFF button is being set.
 * - The ON code has been received - remains lit for 5 seconds
 * - The OFF code has been received - remains lit for 5 seconds
 * 
 * The following peripherals are used:
 *  TIMER0 for the general timer
 *  TIMER3 for the IR pulse receive measurement
 * 
 * The following interrupts are used:
 *  HP for TIMER3 to measure the IR receive pulses
 *  LP for TIMER0 for the general timer
 * 
 * The IR code is based on IRremote
 * Version 0.11 August, 2009, by Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 */

#include <xc.h>
#include "hardware.h"
#include "ticker.h"
#include "irsw.h"
#include "nvm.h"
extern eeprom_data_t EEPROM_Read(eeprom_address_t index);
extern uint8_t EEPROM_Write(eeprom_address_t index, eeprom_data_t value);
#include "ir.h"

// CONFIG1L
#pragma config RETEN =     OFF      // VREG Sleep Enable bit (Ultra low-power regulator is Disabled (Controlled by REGSLP bit))
#pragma config INTOSCSEL = HIGH // LF-INTOSC Low-power Enable bit (LF-INTOSC in High-power mode during Sleep)
#pragma config SOSCSEL =   DIG    // SOSC Power Selection and mode Configuration bits (Digital (SCLKI) mode)
#pragma config XINST =     OFF      // Extended Instruction Set (Disabled)

// CONFIG1H
#pragma config FOSC =      HS1       // Oscillator (HS oscillator (Medium power, 4 MHz - 16 MHz))
#pragma config PLLCFG =    OFF      // PLL x4 Enable bit (Disabled)
#pragma config FCMEN =     OFF      // Fail-Safe Clock Monitor (Disabled)
#pragma config IESO =      OFF       // Internal External Oscillator Switch Over Mode (Disabled)

// CONFIG2L
#pragma config PWRTEN =    ON      // Power Up Timer (Enabled)
#pragma config BOREN =     SBORDIS      // Brown Out Detect (Disabled in hardware, SBOREN disabled)
#pragma config BORV =      0         // Brown-out Reset Voltage bits (3.0V)
#pragma config BORPWR =    ZPBORMV // BORMV Power level (ZPBORMV instead of BORMV is selected)

// CONFIG2H
#pragma config WDTEN =     OFF      // Watchdog Timer (WDT disabled in hardware; SWDTEN bit disabled)
#pragma config WDTPS =     1048576      // Watchdog Postscaler (1:1048576)

// CONFIG3H
#pragma config CANMX =     PORTB    // ECAN Mux bit (ECAN TX and RX pins are located on RB2 and RB3, respectively)
#pragma config MSSPMSK =   MSK7   // MSSP address masking (7 Bit address masking mode)
#pragma config MCLRE =     ON       // Master Clear Enable (MCLR Enabled, RE3 Disabled)

// CONFIG4L
#pragma config STVREN =    ON      // Stack Overflow Reset (Enabled)
#pragma config BBSIZ =     BB1K     // Boot Block Size (1K word Boot Block size)

// CONFIG5L
#pragma config CP0 =       OFF        // Code Protect 00800-01FFF (Disabled)
#pragma config CP1 =       OFF        // Code Protect 02000-03FFF (Disabled)
#pragma config CP2 =       OFF        // Code Protect 04000-05FFF (Disabled)
#pragma config CP3 =       OFF        // Code Protect 06000-07FFF (Disabled)

// CONFIG5H
#pragma config CPB =       OFF        // Code Protect Boot (Disabled)
#pragma config CPD =       OFF        // Data EE Read Protect (Disabled)

// CONFIG6L
#pragma config WRT0 =      OFF       // Table Write Protect 00800-01FFF (Disabled)
#pragma config WRT1 =      OFF       // Table Write Protect 02000-03FFF (Disabled)
#pragma config WRT2 =      OFF       // Table Write Protect 04000-05FFF (Disabled)
#pragma config WRT3 =      OFF       // Table Write Protect 06000-07FFF (Disabled)

// CONFIG6H
#pragma config WRTC =      OFF       // Config. Write Protect (Disabled)
#pragma config WRTB =      OFF       // Table Write Protect Boot (Disabled)
#pragma config WRTD =      OFF       // Data EE Write Protect (Disabled)

// CONFIG7L
#pragma config EBTR0 =     OFF      // Table Read Protect 00800-01FFF (Disabled)
#pragma config EBTR1 =     OFF      // Table Read Protect 02000-03FFF (Disabled)
#pragma config EBTR2 =     OFF      // Table Read Protect 04000-05FFF (Disabled)
#pragma config EBTR3 =     OFF      // Table Read Protect 06000-07FFF (Disabled)

// CONFIG7H
#pragma config EBTRB =     OFF      // Table Read Protect Boot (Disabled)

// forward references
void setup(void);
void loop(void);
unsigned long loadCode(uint8_t codeNo);
void saveCode(uint8_t codeNo, unsigned long code);


// locally used types
typedef enum {
    CONFIG_NONE,
    CONFIG_CODE1,
    CONFIG_CODE2
} ConfigState;

// Global variables
TickValue onTime;
ConfigState configState;
unsigned long code1_hash;
unsigned long code2_hash;
decode_results ir_results;

// Main application

void main(void) {
    uint8_t t1, t2, i;
    /* Introduce a startup delay so that the power supply can stabilise */
    /* Without this EEPROM can get corrupted during power up. A  MCP111-450 
     * dongle does resolve this but is unnecessary with this software fix. 
     * Delay is approx 1 second. */
    for (t1=0; t1<64; t1++) {
        for (t2=0; t2<255; t2++) {
            for (i=0; i<255; i++) {
                // do something innocuous
                BLINKLED_OFF();
            }
        }
    }
    setup();
    
    while (1) {
        loop();
    }

}

void setup(void) {
    // interrupts
    bothDi();
    RCONbits.IPEN = 1;  // enable H and L priorities
    
    initTicker(0); // low priority interrupt
    initRomOps();
    // All Digital ports, no analogue
    ANCON0 = 0;
    ANCON1 = 0;

    // Set up the default port state
    LED_CONFIG1_DIRECTION = OUTPUT;
    LED_CONFIG2_DIRECTION = OUTPUT;
    LED_DATA1_DIRECTION = OUTPUT;
    LED_DATA2_DIRECTION = OUTPUT;
    BLINKLED_DIRECTION = OUTPUT;
    OUTPUT_DIRECTION = OUTPUT;
    CONFIG_SWITCH_DIRECTION = INPUT;
    IR_RECEIVE_DIRECTION = INPUT;
    
    LED_CONFIG1_OFF();
    LED_CONFIG2_OFF();
    LED_DATA1_OFF();
    LED_DATA2_OFF();
    BLINKLED_OFF();
    OUTPUT_CHANNEL_OFF();
    
    // Set up the IR receiver
    ir_enableIRIn();
    
    // set up the application state machine
    configState = CONFIG_NONE;
    
    // load the saved codes from EEPROM
    code1_hash = loadCode(0);
    code2_hash = loadCode(1);
    bothEi();
}

void loop(void) {
    uint8_t ready;
    
    ready = ir_decode(&ir_results);
    
    if (SWITCH_CONFIG) {
        /*** do config ***/

        // Show indicator of config state
        switch (configState) {
            case CONFIG_NONE:
                // just entered config
                // get rid of last codes
                code1_hash = code2_hash = 0;
                configState = CONFIG_CODE1;
                // fall through
            case CONFIG_CODE1:
                LED_CONFIG1_ON();
                LED_CONFIG2_OFF();
                break;
            case CONFIG_CODE2:
                LED_CONFIG1_OFF();
                LED_CONFIG2_ON();
                break;
        }
        if (ready == DECODED) {
            // move to next state
            switch (configState) {
                case CONFIG_CODE1:
                    // make sure we don't save the same code for 1 as that for 2 due to transmitter repetition
                    if (ir_results.value != code2_hash) {
                        code1_hash = ir_results.value;
                        saveCode(0, code1_hash);
                        configState = CONFIG_CODE2;
                    }
                    break;
                case CONFIG_CODE2:
                    // make sure we don't save the same code for 2 as that for 1 due to transmitter repetition
                    if (ir_results.value != code1_hash) {
                        code2_hash = ir_results.value;
                        saveCode(1, code2_hash);
                        configState = CONFIG_CODE1;
                    }
                    break;
                case CONFIG_NONE:
                    // should never get here
                    break;
            }
            ir_resume();
        }
    } else {
        if (ready == DECODED) {
            configState = CONFIG_NONE; // ready for whenever config mode is entered again
            LED_CONFIG1_OFF();
            LED_CONFIG2_OFF();
            if (ir_results.value == code1_hash) {
                LED_DATA1_ON();
                onTime.val = tickGet();
                OUTPUT_CHANNEL_ON();
            }
            if (ir_results.value == code2_hash) {
                LED_DATA2_ON();
                onTime.val = tickGet();
                OUTPUT_CHANNEL_OFF();
            }
            if (tickTimeSince(onTime) > FIVE_SECOND) {
                LED_DATA1_OFF();
                LED_DATA2_OFF();
            }
            ir_resume();
        }
    }
}

/**
 * Read a 4 byte code from EEPROM. Codes are saved with MSB at lowest address.
 * @param code number which code to load
 * @return the code value
 */
unsigned long loadCode(uint8_t codeNo) {
    unsigned long res;
    res =  EEPROM_Read(0 + 4*codeNo);
    res = res << 8;
    res |= EEPROM_Read(1 + 4*codeNo);
    res = res << 8;
    res |= EEPROM_Read(2 + 4*codeNo);
    res = res << 8;
    res |= EEPROM_Read(3 + 4*codeNo);
    return res;
}

/**
 * Save a 4 byte code into EEPROM. Codes are saved with MSB at lowest address.
 * @param code number which code to save
 * @param code the code value to be saved
 */
void saveCode(uint8_t codeNo, unsigned long code) {
    EEPROM_Write(3 + 4*codeNo, code & 0xFF);
    code = code >> 8;
    EEPROM_Write(2 + 4*codeNo, code & 0xFF);
    code = code >> 8;
    EEPROM_Write(1 + 4*codeNo, code & 0xFF);
    code = code >> 8;
    EEPROM_Write(0 + 4*codeNo, code & 0xFF);
}

/*
 * High priority Interrupt Service Routine.
 */ 
void __interrupt(high_priority) __section("mainSec") isrHigh() {
    ir_interruptService();
}

/*
 * Low priority Interrupt Service Routine handler.
 */
void __interrupt(low_priority) __section("mainSec") isrLow() {
    tickerIsr();
}
