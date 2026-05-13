
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef HARDWARE_H
#define	HARDWARE_H

#define clkMHz 16

// The LED outputs
#define LED_CONFIG1_ON()        {LATCbits.LATC1 = 1;}
#define LED_CONFIG1_OFF()       {LATCbits.LATC1 = 0;}
#define LED_CONFIG1_DIRECTION   TRISCbits.TRISC1
#define LED_CONFIG2_ON()        {LATCbits.LATC2 = 1;}
#define LED_CONFIG2_OFF()       {LATCbits.LATC2 = 0;}
#define LED_CONFIG2_DIRECTION   TRISCbits.TRISC2
#define LED_DATA1_ON()          {LATCbits.LATC3 = 1;}
#define LED_DATA1_OFF()         {LATCbits.LATC3 = 0;}
#define LED_DATA1_DIRECTION     TRISCbits.TRISC3
#define LED_DATA2_ON()          {LATCbits.LATC4 = 1;}
#define LED_DATA2_OFF()         {LATCbits.LATC4 = 0;}
#define LED_DATA2_DIRECTION     TRISCbits.TRISC4
#define BLINKLED_ON()           {LATCbits.LATC5 = 1;}
#define BLINKLED_OFF()          {LATCbits.LATC5 = 0;}
#define BLINKLED_DIRECTION      TRISCbits.TRISC5
// The digital output
#define OUTPUT_CHANNEL_ON()     {LATBbits.LATB0 = 1;}
#define OUTPUT_CHANNEL_OFF()    {LATBbits.LATB0 = 0;}
#define OUTPUT_DIRECTION        TRISBbits.TRISB0
// The inputs
#define SWITCH_CONFIG           PORTAbits.RA2
#define CONFIG_SWITCH_DIRECTION TRISAbits.TRISA2
#define IR_RECEIVE_PIN          PORTAbits.RA0
#define IR_RECEIVE_DIRECTION    TRISAbits.TRISA0


#endif	/* LEDS_H */

