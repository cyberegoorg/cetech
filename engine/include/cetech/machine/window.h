#ifndef CETECH_MACHINE_WINDOW_H
#define CETECH_MACHINE_WINDOW_H


#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>
#include <stddef.h>

#include "_machine_enums.h"

//==============================================================================
// Window
//==============================================================================

enum ct_window_flags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum ct_window_pos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};

typedef void ct_window_ints;

struct ct_window {
    ct_window_ints *inst;

    void (*set_title)(ct_window_ints *w,
                      const char *title);

    const char *(*get_title)(ct_window_ints *w);

    void (*update)(ct_window_ints *w);

    void (*resize)(ct_window_ints *w,
                   uint32_t width,
                   uint32_t height);

    void (*size)(ct_window_ints *window,
                 uint32_t *width,
                 uint32_t *height);

    void *(*native_window_ptr)(ct_window_ints *w);

    void *(*native_display_ptr)(ct_window_ints *w);
};


struct ct_window_a0 {
    ct_window *(*create)(struct cel_alloc *alloc,
                         const char *title,
                         enum ct_window_pos x,
                         enum ct_window_pos y,
                         const int32_t width,
                         const int32_t height,
                         enum ct_window_flags flags);

    ct_window *(*create_from)(struct cel_alloc *alloc,
                              void *hndl);

    void (*destroy)(struct cel_alloc *alloc,
                    ct_window *w);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MACHINE_WINDOW_H
