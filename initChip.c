//    initChip.c
#include "initChip.h"

void initWDT(void) {
    //init_WDT

    WDTCON = 0b00010010; //512ms
    // WDTCONbits.SWDTEN=1;
}

void initCPU(void) {
    // SCS FOSC; SPLLEN disabled; IRCF 16MHz_HF;
    OSCCON = 0x78;
    // TUN 0;
    OSCTUNE = 0x00;
    // SBOREN disabled;
    BORCON = 0x00;
}

void initPORTA(void) {
    //init_PORTA

    PORTA = 0x00;
    LATA = 0x00;
    ANSELA = 0x00; // all analog to digital I/O
    TRISA = 0x3F; //all pins input
    INTCON = 0x00; //сброс флага INTF

}

void initPORTC(void) {
    //init_PORTC
    PORTC = 0x00;
    LATC = 0x00;
    ANSELC = 0x00; // all analog to digital I/O
    TRISC = 0x3F; //all pins input
}

void initChip(void) {
    initCPU();
    initWDT();
    initPORTA();
    initPORTC();
}
