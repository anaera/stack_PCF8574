#ifndef I2C_DRIVER_H
#define	I2C_DRIVER_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    I2C_OK = 0,
    I2C_ADDR_ERR,
    I2C_DATA_ERR,
    I2C_BUFF_ERR,
    I2C_FATAL_ERR
} error_e;

typedef struct {
    uint8_t adr; //адрес чипа
    void (*exitFunc)(void); //выход из прерывания
    error_e error; //ошибка операции
    uint8_t value; //данные для обмена
} i2c_api_t;

//API ENGINE
void I2C_Init(void);
void I2C_Open(void);
void I2C_Close(void);
bool I2C_Start(volatile i2c_api_t *i2c_discriptor);
bool I2C_Recover(void);

//IRQ ENGINE
void I2C_Bus_Error(void);
void I2C_MasterStateEngine(void);

#endif	/* I2C_DRIVER */

