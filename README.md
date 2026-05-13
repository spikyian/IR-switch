# IR-switch
PIC based infra-red switch.

Intended to work with any IR transmitted wich used a 38Khz modulated signal. This signal can be demodulated by 
any of the typical IR receiver integrated circuits.

A switch is used to put the code into configuration mode whereupon two IR codes can be sent to the module. These
two codes are stored in EEPROM. When taken out of configuration mode upon receiving the stored codes the output pin
changed from high to low.

Five LED outputs are also provided:
 - Blink whenever an IR code is received.
 - Turn on for 5 seconds when the ON code is received
 - Turn on for 5 seconds when the OFF code is received
 - In configuration mode awaiting the ON code
 - In configuration mode awaiting the OFF code

Written for the PIC18F25K80 processor using XC8.

