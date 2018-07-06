#ifndef CETECH_LEVEL_INSPECTOR_H
#define CETECH_LEVEL_INSPECTOR_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>


struct ct_entity;
struct ct_world;

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Api
//==============================================================================

struct ct_explorer_a0 {
    void (*set_level)(struct ct_world world,
                      struct ct_entity level,
                      uint64_t name,
                      uint64_t root,
                      const char *path);
};

CT_MODULE(ct_explorer_a0);
//CT_MODULE_API struct ct_explorer_a0 ct_explorer_a0;

#endif //CETECH_LEVEL_INSPECTOR_H
