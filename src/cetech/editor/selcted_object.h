#ifndef CETECH_SELECTED_OBJECT_H
#define CETECH_SELECTED_OBJECT_H

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#include "celib/module.inl"

struct ct_selected_object_a0 {
    void (*set_selected_object)(uint64_t context, uint64_t obj);
    uint64_t (*selected_object)(uint64_t context);
};


CE_MODULE(ct_selected_object_a0);

#endif //CETECH_SELECTED_OBJECT_H
