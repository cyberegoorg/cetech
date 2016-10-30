#ifndef CELIB_WINDOW_TYPES_H
#define CELIB_WINDOW_TYPES_H

//==============================================================================
// Mouse
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

#endif //CELIB_WINDOW_TYPES_H
