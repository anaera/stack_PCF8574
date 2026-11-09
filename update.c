#include "i2c_driver.h"
#include "update.h"
#include "epp_full.h"

extern __eeprom ee_t ee;
void (*updateFunc)(void) = 0; /* явное определение + гарантированный NULL до doInitUpdate */

state_pin_t statePin;
volatile i2c_api_t i2c_api;
uint8_t upd_chip; //номер текущего чипа
uint8_t shadow_val[NUM_PCF8574]; //теневое хранилище состояния пинов всех чипов

//CORE  updater
static void doIdleUpdate(void);
static void doWrUpdate(void);
static void doTrailWR(void);
static void doCheckExit(void);
static void doRdUpdate(void);
static void doNextRD(void);
static void doCheckUpdate(void);
static void doRecoveryStep(void);
static void doCheckError(void);


//  задаем начальное значение
//  читаем состояние регистра doRdUpdate()
//  переходим на анализ doNextRD()
//  анализируем doCheckUpdate()
//  записываем новое значение регитров doWrUpdate()
//  переходим на анализ завершения цикла doTrailWR()
//  анализируем завершениу цикла doCheckExit()
//  цикл обновления выходных регистров после чтения и обработки входных

void doResetPCF8574() {
    //TODO делаем аппаратный ресет по питанию только PCF8574
    //такая техническая возможность есть.
}

void doInitUpdate(void) {
    for (upd_chip = 0; upd_chip < NUM_PCF8574; upd_chip++) {
        shadow_val[upd_chip] = ee.ee_chip[upd_chip].val_pin;
    }
    updateFunc = doIdleUpdate;
}

void doStartUpdate(void) {
    upd_chip = 0;
    updateFunc = doRdUpdate;
}

static void doIdleUpdate(void) {
    NOP();
#ifdef I2C_SIM
    /* модель ведомого */
    extern uint8_t sim_exit;
    if (upd_chip < NUM_PCF8574) {
        PIR1bits.SSP1IF = 1;
        return;
    }
    sim_exit = 0;
    NOP();
#endif
}

static void doWrUpdate(void) {
    i2c_api.adr = (uint8_t) (ee.ee_chip[upd_chip].adr_chip << 1);
    i2c_api.error = 0;
    i2c_api.exitFunc = doTrailWR;
    if (I2C_Start(&i2c_api)) {
        updateFunc = doIdleUpdate; //стартанули нормально
    } else {
        updateFunc = doRecoveryStep; //ошибка на старте
    };
}

static void doTrailWR(void) {
    updateFunc = doCheckExit; //нормально, обработка состояния пинов
    doCheckError(); //завершили с ошибкой
}

static void doCheckExit(void) {
    //u.apd[upd_chip] = 0; //сбрасываем update флаги
    upd_chip++; //указатель на следующий чип
    //Для отладки == 0 один регистр (8бит)
    if (upd_chip < NUM_PCF8574) { //upd.act - регистров по факту в будующем
        updateFunc = doRdUpdate; //далее,следующий чип
        return;
    }
    updateFunc = doIdleUpdate; //больше чипов нет, состояние idleUpdate
    return;
}

static void doCheckError(void) {
    switch (i2c_api.error) {
        case I2C_OK:
            break;
        case I2C_ADDR_ERR:
        case I2C_DATA_ERR:
            updateFunc = doCheckExit; // не фатально, идем к следующему чипу
            break;
        case I2C_BUFF_ERR:
        case I2C_FATAL_ERR:
            updateFunc = doRecoveryStep; //требует ремонта PCF8574
            break;
    }
}

static void doRecoveryStep(void) {
    if (I2C_Recover()) { //восстаовилось-пропускаем все остальное и идем на новый цикл
        updateFunc = doIdleUpdate;
    } else {//НЕ восстаовилось- и идем на апппаратную перезагрузку
        updateFunc = doResetPCF8574;
    }
}

static void doRdUpdate(void) { //чтение регистра I2C 
    NOP();
#ifdef I2C_SIM
    /* модель ведомого */
    extern volatile uint8_t sim_rx; /* что "чип" отдал бы на чтение */
    extern volatile bool sim_nack; /* true = ведомый отвечает NACK */
    extern volatile bool sim_wcol; /* true = ошибка записи в буфер */
    sim_nack = (upd_chip == 3); /* чип 3 "мертв" — тест ADDR_ERR */
    sim_wcol = (upd_chip == 4); /* чип 4 ошибка записи в буфер */
    sim_rx = 0x5A; /* конкретное значение для чтения */


#endif
    i2c_api.adr = (uint8_t) (ee.ee_chip[upd_chip].adr_chip << 1) | 1;
    i2c_api.error = 0;
    i2c_api.exitFunc = doNextRD;
    if (I2C_Start(&i2c_api)) {
        updateFunc = doIdleUpdate; //стартовали нормально, ждем завеершения
    } else {
        updateFunc = doRecoveryStep; //ошибка на старте, идем ремонтировать шину 
    }
}

static void doNextRD(void) {
    updateFunc = doCheckUpdate; //обработка состояния пинов
    doCheckError();
}

//прочитанный  из PCF8574 байт i2c_api.value преобразуем в биты для отправки на обработку

static void doCheckUpdate(void) {
    uint8_t upd_val;
    uint8_t mask = 1;
    uint8_t dir = ee.ee_chip[upd_chip].dir_pin; //читаем dir epprom 
    i2c_api.value = i2c_api.value ^ ee.ee_chip[upd_chip].inv_pin; //инверсные биты
    upd_val = i2c_api.value ^ shadow_val[upd_chip]; //биты которые изменились
    //новые, которые входные берем от i2c_api.value, которые выходные уже в pin_val
    // изменены напрямую приложением и ждут записи
    shadow_val[upd_chip] = (i2c_api.value & dir) | (shadow_val[upd_chip] & (~dir)); //новое значение pin
    i2c_api.value = shadow_val[upd_chip] | dir; //пишем в новые в OUP пин и 0xFF в INP
    if (upd_val != 0) { //есть изменения
        uint8_t num_pin = (uint8_t) (upd_chip << 3); //считаем num 
        while (mask) {
            if (mask & upd_val) { //изменился ли пин
                statePin.number = num_pin++; //его номер и далее 
                statePin.state = (shadow_val[upd_chip] & mask) ? _UP : _DN; //его значение
                sendState(statePin); //шлем в PDO
                if (mask & ee.ee_chip[upd_chip].btn_pin) checkPress(statePin); //длина нажатия
            }
            mask = (uint8_t) (mask << 1); //следующий пин

        }
    }
    updateFunc = doWrUpdate; //далее,следующий чип
}

void checkPress(state_pin_t statePin) {
    //отсылка пина на обработку длительности (входные)
    NOP();
}

void sendState(state_pin_t statePin) {
    //отсылка пина на в обработку отправки (выодные)
    NOP();
}

//пины пишем и читаем через теневое хранилище состояния пинов pin_val

void writePin(state_pin_t statePin) {
    if (statePin.act) return; //бит длиетельности - ничего не делаем
    uint8_t mask = (uint8_t) (1 << statePin.bit);
    uint8_t num_chip = statePin.chip;

    if (mask & ee.ee_chip[num_chip].dir_pin)return; //inp пин - ничего не делаем
    if (statePin.val) {
        shadow_val[num_chip] = shadow_val[num_chip] | mask; //устанавливаем в 1
    } else {
        shadow_val[num_chip] = shadow_val[num_chip] & (~mask); //устанавливаем в 0
    }

}

bool readPin(state_pin_t statePin) {
    uint8_t mask = (uint8_t) (1 << statePin.bit);
    uint8_t num_chip = statePin.chip;
    return (shadow_val[num_chip] & mask);
}