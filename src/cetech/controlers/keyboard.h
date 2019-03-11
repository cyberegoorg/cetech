#ifndef CETECH_KEYBOARD_H
#define CETECH_KEYBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CONTROLER_KEYBOARD \
    CE_ID64_0("keyboard", 0x413d83f1d027f82eULL)

#define CT_KEYBOARD_TASK \
    CE_ID64_0("keyboard_task", 0x905ae7202b55aafcULL)

#ifdef __cplusplus
};
#endif

#endif //CETECH_KEYBOARD_H
//! \}