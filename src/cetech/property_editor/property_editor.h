#ifndef CETECH_PROPERTY_INSPECTOR_H
#define CETECH_PROPERTY_INSPECTOR_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Api
//==============================================================================

#define PROPERTY_EDITOR_INTERFACE_NAME \
    "ct_property_editor_i0"

#define PROPERTY_EDITOR_INTERFACE \
    CT_ID64_0("ct_property_editor_i0", 0x50566f03b5eacb95ULL)


struct ct_property_editor_i0 {
    void (*draw_ui)();
};


#endif //CETECH_PROPERTY_INSPECTOR_H
