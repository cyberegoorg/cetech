#ifndef CETECH_MOUSE_H
#define CETECH_MOUSE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define CONTROLER_MOUSE \
    CE_ID64_0("mouse", 0x7c351278fe937928ULL)

#define CT_MOUSE_TASK \
    CE_ID64_0("mouse_task", 0x143aa2eadb0ac19eULL)

#ifdef __cplusplus
};
#endif

#endif //CETECH_MOUSE_H
