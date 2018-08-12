#ifndef CETECH_DOCK_H
#define CETECH_DOCK_H

#include <stddef.h>
#include <stdint.h>

enum DebugUIWindowFlags_;

#define DOCK_INTERFACE_NAME \
    "ct_dock_i0"

#define DOCK_INTERFACE \
    CE_ID64_0("ct_dock_i0", 0x3a3181c12a9e109fULL)

struct ct_dock_i0 {
    uint64_t id;
    bool visible;
    enum DebugUIWindowFlags_ dock_flag;

    const char* (*display_title)(struct ct_dock_i0* dock);
    const char* (*name)(struct ct_dock_i0* dock);

    void (*draw_ui)(struct ct_dock_i0* dock);
    void (*draw_menu)(struct ct_dock_i0* dock);
    void (*draw_main_menu)();
};

#endif //CETECH_DOCK_H
