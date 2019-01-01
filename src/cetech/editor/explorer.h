#ifndef CETECH_EXPLORER_H
#define CETECH_EXPLORER_H

#include <stdint.h>

#define EXPLORER_EBUS \
    CE_ID64_0("explorer", 0xcf28ca0dccf09520ULL)

#define EXPLORER_INTERFACE \
    CE_ID64_0("ct_explorer_i0", 0xdaa9b6ac2664c06dULL)

struct ct_explorer_i0 {
    uint64_t (*cdb_type)();
    uint64_t (*draw_ui)(uint64_t top_level_obj, uint64_t selected_obj);
    void (*draw_menu)(uint64_t selected_obj);
};

#endif //CETECH_EXPLORER_H
