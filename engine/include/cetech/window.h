#ifndef CETECH_WINDOW_H
#define CETECH_WINDOW_H


//==============================================================================
// Window
//==============================================================================

enum {
    WINDOW_API_ID = 231515643
};

typedef struct window_s {
    void *w;
} window_t;

enum window_flags {
    WINDOW_NOFLAG = 0,
    WINDOW_FULLSCREEN = 1,
};

enum window_pos {
    WINDOWPOS_CENTERED = 1,
    WINDOWPOS_UNDEFINED = 2
};


//==============================================================================
// Interface
//==============================================================================

struct window_api_v0 {
    window_t (*create)(const char *title,
                           enum window_pos x,
                           enum window_pos y,
                           const int32_t width,
                           const int32_t height,
                           enum window_flags flags);

    window_t (*create_from)(void *hndl);

    void (*destroy)(window_t w);

    void (*set_title)(window_t w,
                      const char *title);

    const char *(*get_title)(window_t w);

    void (*update)(window_t w);

    void (*resize)(window_t w,
                   uint32_t width,
                   uint32_t height);

    void (*get_size)(window_t window,
                     uint32_t *width,
                     uint32_t *height);

    void *(*native_window_ptr)(window_t w);

    void *(*native_display_ptr)(window_t w);
};

#endif //CETECH_WINDOW_H
