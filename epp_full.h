#ifndef EPP_FULL_H
#define	EPP_FULL_H
#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include "epp_update.h"

typedef struct {
    state_chip_t ee_chip [NUM_PCF8574];
} ee_t;

#endif	/* EPP_FULL_H */

