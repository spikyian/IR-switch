
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
#define LED_CONFIG1_ON()        {LATBbits.LATB6 = 1;}
#define LED_CONFIG1_OFF()       {LATBbits.LATB6 = 0;}

#define LED_CONFIG2_ON()        {LATBbits.LATB7 = 1;}
#define LED_CONFIG2_OFF()       {LATBbits.LATB7 = 0;}


#define LED_DATA1_ON()          {LATCbits.LATC6 = 1;}
#define LED_DATA1_OFF()         {LATCbits.LATC6 = 0;}

#define LED_DATA2_ON()          {LATCbits.LATC5 = 1;}
#define LED_DATA2_OFF()         {LATCbits.LATC5 = 0;}

#define BLINKLED_ON()           {LATCbits.LATC7 = 1;}
#define BLINKLED_OFF()          {LATCbits.LATC7 = 0;}

// The digital output
#define OUTPUT_CHANNEL_ON()     {LATBbits.LATB2 = 1;}
#define OUTPUT_CHANNEL_OFF()    {LATBbits.LATB2 = 0;}

// The inputs
#define SWITCH_CONFIG           (PORTAbits.RA2 == 0)
#define IR_RECEIVE_PIN          PORTCbits.RC0



#endif	/* LEDS_H */

