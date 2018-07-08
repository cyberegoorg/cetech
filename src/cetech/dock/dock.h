#ifndef CETECH_DOCK_H
#define CETECH_DOCK_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

enum DebugUIWindowFlags_;

struct ct_dock_i0 {
    uint64_t id;
    bool visible;
    enum DebugUIWindowFlags_ dock_flag;

    const char* (*display_title)(struct ct_dock_i0* dock);
    const char* (*name)(struct ct_dock_i0* dock);

    void (*draw_ui)(struct ct_dock_i0* dock);
    void (*draw_main_menu)();
};

#endif //CETECH_DOCK_H
