#ifndef CETECH_SELECTED_OBJECT_H
#define CETECH_SELECTED_OBJECT_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

#include <celib/module.inl>


//==============================================================================
// Api
//==============================================================================

struct ct_selected_object_a0 {
    uint64_t (*selected_object)();
    void (*set_selected_object)(uint64_t object);
};

CE_MODULE(ct_selected_object_a0);

#endif //CETECH_SELECTED_OBJECT_H
