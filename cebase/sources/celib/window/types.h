#ifndef CELIB_WINDOW_TYPES_H
#define CELIB_WINDOW_TYPES_H

//==============================================================================
// Mouse
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

#endif //CELIB_WINDOW_TYPES_H
