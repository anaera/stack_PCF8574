#ifndef I2C_CORE_H
#define	I2C_CORE_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#define _XTAL_FREQ 16000000UL /* для __delay_us */

typedef enum {
    _IDLE = 0,
    _AFTER_SEND_START,
    _AFTER_SEND_ADDR,
    _AFTER_SEND_DATE,
    _AFTER_SEND_RxEN,
    _AFTER_SEND_NAK,
    _AFTER_SEND_STOP
} state_e;

//-----------------------------------------------------------------------------

static inline void I2C_MasterOpen(void) {
    SSP1CON1bits.SSPEN = 1;
}

static inline void I2C_MasterClose(void) {
    SSP1CON1bits.SSPEN = 0;
}

static inline bool I2C_MasterBusReady(void) {
    return !SSP1STATbits.S && SSP1CON1bits.SSPEN && PORTCbits.RC1;
}

static inline void I2C_MasterSendStart(void) {
    SSP1CON2bits.SEN = 1;
}

static inline void I2C_MasterSendTxData(uint8_t data) {
    SSP1BUF = data;
}

static inline bool I2C_MasterRecvIsNack(void) {
    return SSP1CON2bits.ACKSTAT;
}

static inline void I2C_MasterSendNack(void) {
    SSP1CON2bits.ACKDT = 1;
    SSP1CON2bits.ACKEN = 1;
}
//резерв под multi-byte read

static inline void I2C_MasterSendAck(void) {
    SSP1CON2bits.ACKDT = 0;
    SSP1CON2bits.ACKEN = 1;
}

static inline void I2C_MasterSendRxEN(void) {
    SSP1CON2bits.RCEN = 1;
}

static inline uint8_t I2C_MasterGetRxData(void) {
    return SSP1BUF;
}

static inline void I2C_MasterSendStop(void) {
    SSP1CON2bits.PEN = 1;
}

static inline void I2C_MasterEnableIRQ(void) {
    PIR1bits.SSP1IF = 0;
    PIE1bits.SSP1IE = 1; //раскомментировать для IRQ крежима
}

static inline void I2C_MasterDisableIRQ(void) {
    // PIE1bits.SSP1IE = 0; //раскомментировать для IRQ крежима
}

static inline bool I2C_MasterIsIRQ(void) {
    return PIR1bits.SSP1IF;
}

static inline void I2C_MasterClearIRQ(void) {
    PIR1bits.SSP1IF = 0; //сброс IRQ
}

static inline void I2C_MasterEnableBCL(void) {
    PIR2bits.BCL1IF = 0;
    PIE2bits.BCL1IE = 1;
}

static inline bool I2C_MasterIsBCL(void) {
    return PIR2bits.BCL1IF;
}

static inline void I2C_MasterClearBCL(void) {
    PIR2bits.BCL1IF = 0;
}

static inline bool I2C_MasterTxOK(void) {
    if (SSP1CON1bits.WCOL) { /* коллизия записи */
        SSP1CON1bits.WCOL = 0; /* сбросить вручную */
        return false;
    }
    return true;
}
#endif	/* I2C_CORE */

