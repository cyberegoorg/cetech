#ifndef CETECH_LEVEL_INSPECTOR_H
#define CETECH_LEVEL_INSPECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>


struct ct_entity;
struct ct_world;

#define EXPLORER_EBUS_NAME "entity_explorer"
#define EXPLORER_EBUS CT_ID64_0(EXPLORER_EBUS_NAME)

enum {
    EXPLORER_INAVLID_EVENT = 0,
    EXPLORER_ENTITY_SELECT_EVENT,
};

//==============================================================================
// Typedefs
//==============================================================================
struct ct_ent_selected_ev {
    struct ct_world world;
    struct ct_entity entity;
    const char *filename;
    ct_cdb_obj_t* obj;
};;

//==============================================================================
// Api
//==============================================================================

struct ct_explorer_a0 {
    void (*set_level)(ct_world world,
                      ct_entity level,
                      uint64_t name,
                      uint64_t root,
                      const char *path);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_LEVEL_INSPECTOR_H
