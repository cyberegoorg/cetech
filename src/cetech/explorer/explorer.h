#ifndef CETECH_EXPLORER_H
#define CETECH_EXPLORER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CT_EXPLORER_I0_STR "ct_explorer_i0"

#define CT_EXPLORER_I0 \
    CE_ID64_0("ct_explorer_i0", 0xdaa9b6ac2664c06dULL)


typedef struct ct_explorer_i0 {
    uint64_t (*cdb_type)();

    uint64_t (*draw_ui)(uint64_t top_level_obj,
                        uint64_t selected_obj,
                        uint64_t context);

    void (*draw_menu)(uint64_t selected_obj,
                      uint64_t context);
} ct_explorer_i0;

#ifdef __cplusplus
};
#endif

#endif //CETECH_EXPLORER_H
