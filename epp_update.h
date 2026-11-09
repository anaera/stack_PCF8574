#ifndef EPP_UPDATE_H
#define	EPP_UPDATE_H

#define NUM_PCF8574 8

typedef struct {
    uint8_t adr_chip; //адрес чипа
    uint8_t val_pin; // начальное состояние пинов
    uint8_t inv_pin; //инверсные пины
    uint8_t dir_pin; //направление пинов
    uint8_t btn_pin; //пины кнопки
} state_chip_t;

#endif	/* EPP_UPDATE_H */

