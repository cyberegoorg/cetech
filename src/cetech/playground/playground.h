#ifndef CETECH_PLAYGROUND_H
#define CETECH_PLAYGROUND_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

struct ct_playground_module_i0 {
    bool (*init)();
    bool (*shutdown)();
    void (*update)(float dt);
    void (*render)();
};

#endif //CETECH_PLAYGROUND_H
