#ifndef CETECH_WINDOWSYSTEM_H
#define CETECH_WINDOWSYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/types.h>

//==============================================================================
// Typedefs
//==============================================================================

typedef struct {
    void *w;
} window_t;

enum WindowFlags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum WindowPos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};

//==============================================================================
// Interface
//==============================================================================

void windowsys_init();

void windowsys_shutdown();


window_t windowsys_new_window(const char *title,
                              enum WindowPos x,
                              enum WindowPos y,
                              const i32 width,
                              const i32 height,
                              enum WindowFlags flags);

window_t windowsys_new_from(void *hndl);

void windowsys_destroy_window(window_t w);

void windowsys_set_title(window_t w,
                         const char *title);

const char *windowsys_get_title(window_t w);

void windowsys_update(window_t w);

void windowsys_resize(window_t w,
                      uint32_t width,
                      uint32_t height);

#endif //CETECH_WINDOWSYSTEM_H
