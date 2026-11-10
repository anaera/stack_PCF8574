//    epp.c
#include "epp_full.h"

__eeprom ee_t ee = {
    .ee_chip =
    {
        [0] =
        { .adr_chip = 0x20, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [1] =
        { .adr_chip = 0x21, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [2] =
        { .adr_chip = 0x22, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [3] =
        { .adr_chip = 0x23, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [4] =
        { .adr_chip = 0x24, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [5] =
        { .adr_chip = 0x25, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [6] =
        { .adr_chip = 0x26, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00},
        [7] =
        { .adr_chip = 0x27, .val_pin = 0xFF, .inv_pin = 0x00, .dir_pin = 0x00, .btn_pin = 0x00}
    }
};
