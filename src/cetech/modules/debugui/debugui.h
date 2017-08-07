#ifndef CETECH_DEBUGUI_H
#define CETECH_DEBUGUI_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>


//==============================================================================
// Api
//==============================================================================

//! DebugUI API V0
struct ct_debugui_a0 {
    void (*render)();

    void (*register_on_gui)(void (*on_gui)());
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_DEBUGUI_H

//! |}