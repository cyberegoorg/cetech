#ifndef CETECH_GAMEPAD_H
#define CETECH_GAMEPAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CONTROLER_GAMEPAD \
    CE_ID64_0("gamepad", 0x6aa699de487f71eeULL)

#define CT_GAMEPAD_TASK \
    CE_ID64_0("gamepad_task", 0x120510962813be6fULL)


#ifdef __cplusplus
};
#endif

#endif //CETECH_GAMEPAD_H
//! \}