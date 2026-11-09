#ifndef UPDATE_H
#define	UPDATE_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "epp_update.h"
#include "epp_full.h"

#define _UP 0x80    //состояние 1
#define _DN 0x00    //состояние 0

typedef union {
    uint8_t total;

    struct {
        uint8_t number : 6;
        uint8_t state : 2;
    };

    struct {
        uint8_t bit : 3; //номер пина 0-7
        uint8_t chip : 3; //номер чипа 0-7
        uint8_t act : 1; //тип значения- величина (0,1), длительность(кор,длин))
        uint8_t val : 1; // значение пина 
    };
} state_pin_t;

// API updater
extern void (*updateFunc)(void);
void doInitUpdate(void);
void doStartUpdate(void);

void checkPress(state_pin_t statePin);
void sendState(state_pin_t statePin);
void writePin(state_pin_t statePin);
bool readPin(state_pin_t statePin);
#endif	/* UPDATE_H */

