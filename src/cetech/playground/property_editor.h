#ifndef CETECH_PROPERTY_INSPECTOR_H
#define CETECH_PROPERTY_INSPECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

//==============================================================================
// Typedefs
//==============================================================================
typedef void (*ct_pi_on_debugui)();

//==============================================================================
// Api
//==============================================================================

struct ct_property_editor_a0 {
    void (*set_active)(ct_pi_on_debugui on_debugui);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PROPERTY_INSPECTOR_H
