#ifndef CELIB_WINDOW_H
#define CELIB_WINDOW_H

#include "cetech/types.h"


//==============================================================================
// Window
//==============================================================================

typedef struct {
    void *w;
} cel_window_t;

enum cel_window_flags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum cel_window_pos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};


//==============================================================================
// Interface
//==============================================================================

cel_window_t cel_window_new(const char *title,
                            enum cel_window_pos x,
                            enum cel_window_pos y,
                            const int32_t width,
                            const int32_t height,
                            enum cel_window_flags flags);

cel_window_t cel_window_new_from(void *hndl);

void cel_window_destroy(cel_window_t w);

void cel_window_set_title(cel_window_t w,
                          const char *title);

const char *cel_window_get_title(cel_window_t w);

void cel_window_update(cel_window_t w);

void cel_window_resize(cel_window_t w,
                       uint32_t width,
                       uint32_t height);


void cel_window_get_size(cel_window_t window,
                         uint32_t *width,
                         uint32_t *height);

void *cel_window_native_cel_window_ptr(cel_window_t w);

void *cel_window_native_display_ptr(cel_window_t w);

#endif //CELIB_WINDOW_H
