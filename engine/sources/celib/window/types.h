#ifndef CETECH_WINDOW_TYPES_H
#define CETECH_WINDOW_TYPES_H

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

#endif //CETECH_WINDOW_TYPES_H
