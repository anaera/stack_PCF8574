#include "update.h"
#include "i2c_driver.h"
#include "initChip.h"

void INT_Start(void) {
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

}

void __interrupt() i2c_INT(void) {
    if (INTCONbits.PEIE == 1) {
        if (PIE2bits.BCL1IE == 1 && PIR2bits.BCL1IF == 1) {
            I2C_Bus_Error();
        }
        if (PIE1bits.SSP1IE == 1 && PIR1bits.SSP1IF == 1) {
            I2C_MasterStateEngine();
        }
    }
}

uint8_t sim_exit = 1;

void main(void) {
    NOP();
    NOP();
    doInitUpdate();
    initChip();
    I2C_Init();
    I2C_Open();
    doStartUpdate();
    INT_Start();
    while (sim_exit) {
        updateFunc();
        CLRWDT();
    }
    I2C_Close();
    return;
}
