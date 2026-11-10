
#ifndef I2C_CODE_SIM_H
#define	I2C_CODE_SIM_H
/* i2c_core_sim.h — включается только в отладочной конфигурации */
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

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

/* модель ведомого */
volatile uint8_t sim_rx; /* что "чип" отдал бы на чтение */
volatile bool sim_nack; /* true = ведомый отвечает NACK */
volatile bool sim_wcol; /* true = ошибка записи в буфер */

static inline void I2C_MasterOpen(void) {
    SSP1CON1bits.SSPEN = 1;
}

static inline void I2C_MasterClose(void) {
    SSP1CON1bits.SSPEN = 0;
}

static inline bool I2C_MasterBusReady(void) {
    return true;
}

static inline void I2C_MasterSendStart(void) {
    SSP1CON2bits.SEN = 1;
    //PIR1bits.SSP1IF = 1;
}

static inline void I2C_MasterSendTxData(uint8_t d) {
    sim_rx = d;
    //PIR1bits.SSP1IF = 1;
}

static inline bool I2C_MasterRecvIsNack(void) {
    return sim_nack;
}

static inline void I2C_MasterSendNack(void) {
    //PIR1bits.SSP1IF = 1;
}

static inline void I2C_MasterSendAck(void) {
    //PIR1bits.SSP1IF = 1;
}

static inline void I2C_MasterSendRxEN(void) {
    //PIR1bits.SSP1IF = 1;
}

static inline uint8_t I2C_MasterGetRxData(void) {
    return sim_rx;
}

static inline void I2C_MasterSendStop(void) {
    //PIR1bits.SSP1IF = 1;
}

static inline void I2C_MasterEnableIRQ(void) {
    PIR1bits.SSP1IF = 0;
    PIE1bits.SSP1IE = 1;
}

static inline void I2C_MasterDisableIRQ(void) {
    PIE1bits.SSP1IE = 0;
}

static inline bool I2C_MasterIsIRQ(void) {
    return PIR1bits.SSP1IF;
}

static inline void I2C_MasterClearIRQ(void) {
    PIR1bits.SSP1IF = 0;
}

static inline void I2C_MasterEnableBCL(void) {
    PIR2bits.BCL1IF = 0;
    PIE2bits.BCL1IE = 1;
}

static inline void I2C_MasterDisableBCL(void) {
    PIE2bits.BCL1IE = 0;
}

static inline bool I2C_MasterIsBCL(void) {
    return PIR2bits.BCL1IF;
}

static inline void I2C_MasterClearBCL(void) {
    PIR2bits.BCL1IF = 0;
}

static inline bool I2C_MasterTxOK(void) {
   return !sim_wcol;
}
#endif	/* I2C_CODE_SIM_H */

