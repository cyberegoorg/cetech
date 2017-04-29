#ifndef CELIB_WINDOW_H
#define CELIB_WINDOW_H

#include "cetech/types.h"


//==============================================================================
// Window
//==============================================================================

enum {
    WINDOW_API_ID = 231515643
};

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

struct window_api_v0 {
    cel_window_t (*create)(const char *title,
                           enum cel_window_pos x,
                           enum cel_window_pos y,
                           const int32_t width,
                           const int32_t height,
                           enum cel_window_flags flags);

    cel_window_t (*create_from)(void *hndl);

    void (*destroy)(cel_window_t w);

    void (*set_title)(cel_window_t w,
                      const char *title);

    const char *(*get_title)(cel_window_t w);

    void (*update)(cel_window_t w);

    void (*resize)(cel_window_t w,
                   uint32_t width,
                   uint32_t height);

    void (*get_size)(cel_window_t window,
                     uint32_t *width,
                     uint32_t *height);

    void *(*native_window_ptr)(cel_window_t w);

    void *(*native_display_ptr)(cel_window_t w);
};

#endif //CELIB_WINDOW_H
