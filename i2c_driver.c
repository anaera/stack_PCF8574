//    i2c_driver.c
#ifdef I2C_SIM
#include  "i2c_core_sim.h"
#else
#include "i2c_core.h"
#endif

#include "i2c_driver.h"

static volatile state_e state; //состояние автомата
static volatile i2c_api_t *ptr_i2c; //указатель на дескриптор i2c драйвера

//-----------------------------------------------------------------------------
void I2C_Init(void) {
    //чтоб не было вопросов ANSELC RC0, RC1 
    ANSELC &= 0xFC; // RC0 и RC1 digital I/O 
    TRISCbits.TRISC0 = 1; //SCL
    TRISCbits.TRISC1 = 1; //SDA
    SSP1STAT = 0x00;
    SSP1CON1 = 0x08;
    SSP1CON2 = 0x00;
    SSP1STATbits.SMP = 1;
    SSP1ADD = 0x27; //= 39; //100кгц=Fi2c = Fosc/(4*(SSP1ADD+1));
    // 16000000/(4*(39+1) = 100000
    SSP1CON1bits.SSPEN = 0;
    I2C_MasterClearBCL();
    I2C_MasterClearIRQ();
    ptr_i2c = 0;
}

void I2C_Open(void) {
    state = _IDLE;
    I2C_MasterOpen();
    I2C_MasterEnableIRQ();
    I2C_MasterEnableBCL();
}

void I2C_Close(void) {
    I2C_MasterDisableIRQ();
    I2C_MasterDisableBCL();
    I2C_MasterClose();

}

bool I2C_Start(volatile i2c_api_t *i2c_discriptor) {
    //if (SSP1STATbits.S || !SSP1CON1bits.SSPEN || !PORTCbits.RC1) return false;
    if (!I2C_MasterBusReady()) return false;
    /* гигиена, не оборона - гасим висячий флаг до взведения state */
    I2C_MasterClearIRQ();
    ptr_i2c = i2c_discriptor;
    ptr_i2c->error = 0;
    state = _AFTER_SEND_START;
    I2C_MasterSendStart();
    return true;
}
//------------------------------------------------------------------------------

static void after_send_START(void) {
    I2C_MasterSendTxData(ptr_i2c->adr);
    if (I2C_MasterTxOK()) {
        state = _AFTER_SEND_ADDR;
    } else {
        ptr_i2c->error = I2C_BUFF_ERR;
        I2C_MasterSendStop();
        state = _AFTER_SEND_STOP;
        return;
    }
}

static void after_send_ADDR(void) {
    if (I2C_MasterRecvIsNack()) { //ответ NAK
        ptr_i2c->error = I2C_ADDR_ERR;
        I2C_MasterSendStop();
        state = _AFTER_SEND_STOP;
        return;
    }
    // ответ AСK
    if (ptr_i2c->adr & 0x01) //далее запись или чтение
    { //чтение
        I2C_MasterSendRxEN();
        state = _AFTER_SEND_RxEN;
    } else { //запись
        I2C_MasterSendTxData(ptr_i2c->value);
        if (I2C_MasterTxOK()) {
            state = _AFTER_SEND_DATE;
        } else {
            ptr_i2c->error = I2C_BUFF_ERR;
            I2C_MasterSendStop();
            state = _AFTER_SEND_STOP;
            return;
        }
    }
}

static void after_send_DATE(void) {
    if (I2C_MasterRecvIsNack()) {
        ptr_i2c->error = I2C_DATA_ERR;
    }
    I2C_MasterSendStop();
    state = _AFTER_SEND_STOP;

}

static void after_send_RxEN(void) {
    ptr_i2c->value = I2C_MasterGetRxData();
    I2C_MasterSendNack();
    state = _AFTER_SEND_NAK;
}

static void after_send_NAK(void) {
    I2C_MasterSendStop();
    state = _AFTER_SEND_STOP;
}

static void do_EXIT(void) {
    if (ptr_i2c != 0) {
        if (ptr_i2c->exitFunc != 0) ptr_i2c->exitFunc();
        ptr_i2c->exitFunc = 0; //это уже лишнее, в  exitFunc уж обработали удалить?
        ptr_i2c = 0; /* транзакции больше нет — указателя тоже нет */
    }
    state = _IDLE;
}

static void do_IDLE(void) {
    NOP();
}

void I2C_Bus_Error(void) {

    I2C_MasterClearBCL();
    if (state == _IDLE || ptr_i2c == 0) return; /* висячий BCL — транзакции нет */

    ptr_i2c->error = I2C_FATAL_ERR;
    do_EXIT(); /* транзакцию оборвать; шину разрулит recover при следующем Start */

}

void I2C_MasterStateEngine(void) {
    I2C_MasterClearIRQ();

    switch (state) {

        case _AFTER_SEND_START:
        {
            after_send_START(); //отсылаем адрес
            return;
        }
        case _AFTER_SEND_ADDR:
        {
            after_send_ADDR(); // контроль ACK после отсылки адреса
            return;
        }
        case _AFTER_SEND_DATE:

        {
            after_send_DATE(); //контроль ACK после отсылки данных
            return;
        }
        case _AFTER_SEND_RxEN:
        {
            after_send_RxEN(); // забираем данные и шлем NAK
            return;
        }
        case _AFTER_SEND_NAK:
        {
            after_send_NAK(); //шлем STOP
            return;
        }
        case _AFTER_SEND_STOP:
        {
            do_EXIT(); // выход funExit() -> set IDLE
            return;
        }
        case _IDLE:
        {
            do_IDLE();
            return;
        }
    }
}

/* Вернуть шину в рабочее состояние без снятия питания.
Вызывать, если перед I2C_Start обнаружена занятая шина
(SSPSTATbits.S == 1 или SDA прижата при свободной логике). */
bool I2C_Recover(void) {
    uint8_t n;

    SSP1CON1bits.SSPEN = 0; /* модуль отпускает пины */

    TRISCbits.TRISC1 = 1; /* SDA — вход (отпущена) */
    TRISCbits.TRISC0 = 0; /* SCL — выход, тактируем вручную */
    LATCbits.LATC0 = 1; /* стартовое SCL = 1 */

    for (n = 0; n < 9; n++) {
        if (PORTCbits.RC1) { /* SDA уже отпущена ведомым */
            break;
        }
        LATCbits.LATC0 = 0; /* низ */
        __delay_us(5); /* полупериод 100 кГц */
        LATCbits.LATC0 = 1; /* высокий — по этому фронту ведомый сдвигается */
        __delay_us(5);
    }

    if (!PORTCbits.RC1) {
        /* SDA не отпустилась за 9 импульсов — аппаратная проблема.
        Зафиксировать ошибку; помогает только перезапуск питания. */
        TRISCbits.TRISC0 = 1;
        return false;
    }

    /* Ручной STOP: SDA low -> high при SCL = 1 */
    TRISCbits.TRISC1 = 0; /* SDA — выход */
    LATCbits.LATC1 = 0; /* прижимаем SDA */
    __delay_us(5);
    TRISCbits.TRISC1 = 1; /* отпускаем: подтяжка поднимает SDA = STOP */
    __delay_us(5);

    TRISCbits.TRISC0 = 1; /* SCL обратно на вход */
    PIR1bits.SSP1IF = 0; /* хвостовые флаги — под ноль */
    SSP1CON1bits.SSPEN = 1; /* модуль снова владеет пинами */

    if (!PORTCbits.RC1) { /* SDA так и прижата ведомым */
        return false; /* SSPEN остаётся 0 — модуль не включён */
    }
    return true;
}