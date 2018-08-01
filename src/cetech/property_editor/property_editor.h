#ifndef CETECH_PROPERTY_INSPECTOR_H
#define CETECH_PROPERTY_INSPECTOR_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

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
    uint64_t (*cdb_type)();
    void (*draw_ui)(uint64_t obj);
};


struct ct_property_editor_a0 {
    void (*draw)(uint64_t obj);
};

CT_MODULE(ct_property_editor_a0);

#endif //CETECH_PROPERTY_INSPECTOR_H
