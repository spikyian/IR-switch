
#define GRSP_OK 0
#define GRSP_UNKNOWN_NVM_TYPE 1

#define bothDi() {INTCONbits.GIEH = INTCONbits.GIEL = 0;}
#define bothEi() {INTCONbits.GIEH = INTCONbits.GIEL = 1;}
#define geti()   (INTCONbits.GIEH | INTCONbits.GIEL)