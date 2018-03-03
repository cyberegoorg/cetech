#ifndef CETECH_WORLD_H
#define CETECH_WORLD_H


#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>
#include <cetech/core/cdb/cdb.h>

struct ct_entity_compile_output;
struct ct_compilator_api;
struct ct_cdb_obj_t;

//==============================================================================
// Enums
//==============================================================================


//==============================================================================
// Typedefs
//==============================================================================

//! World handler
struct ct_world {
    uint64_t h;
};

//! Entity typedef
struct ct_entity {
    uint64_t h;
};

//! Component compiler
//! \param body Component body yaml
//! \param data Compiled compoent data
typedef int (*ct_component_compiler_t)(const char *filename,
                                       uint64_t *component_key,
                                       uint32_t component_key_count,
                                       struct ct_cdb_obj_t *writer);

//==============================================================================
// Structs
//==============================================================================

//! World callbacks
typedef struct {
    //! On world create callback
    //! \param world World
    void (*on_created)(struct ct_world world);

    //! On world destroy callback
    //! \param world World
    void (*on_destroy)(struct ct_world world);

} ct_world_callbacks_t;

typedef void (*ct_simulate_fce_t)(struct ct_world world,
                                  struct ct_entity *ent,
                                  struct ct_cdb_obj_t **obj,
                                  uint32_t n,
                                  float dt);

struct ct_comp_watch {
    void (*on_add)(struct ct_world world,
                   struct ct_entity ent,
                   uint64_t comp_mask);

    void (*on_remove)(struct ct_world world,
                      struct ct_entity ent,
                      uint64_t comp_mask);
};

//==============================================================================
// Api
//==============================================================================


struct ct_world_a0 {
    // WORLD
    struct ct_world (*create_world)();

    void (*destroy_world)(struct ct_world world);

    void (*register_world_callback)(ct_world_callbacks_t clb);

    // ENT
    void (*create_entity)(struct ct_world world, struct ct_entity * entity, uint32_t count);

    void (*destroy_entity)(struct ct_world world,
                           struct ct_entity *entity,
                           uint32_t count);

    struct ct_cdb_obj_t *(*ent_obj)(struct ct_world world,
                                    struct ct_entity entity);

    bool (*entity_alive)(struct ct_world world,
                         struct ct_entity entity);

    struct ct_entity (*spawn_entity)(struct ct_world world,
                                     uint32_t name);

    struct ct_entity (*find_by_uid)(struct ct_world world,
                                    struct ct_entity root,
                                    uint64_t uid);


    // COMPONENT
    void (*register_component)(const char *component_name);

    void (*register_component_compiler)(uint64_t type,
                                        ct_component_compiler_t compiler);


    uint64_t (*component_mask)(uint64_t component_name);

    bool (*has)(struct ct_world world,
                struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count);

    void (*add_components)(struct ct_world world,
                           struct ct_entity ent,
                           uint64_t *component_name,
                           uint32_t name_count);

    void (*remove_components)(struct ct_world world,
                              struct ct_entity ent,
                              uint64_t *component_name,
                              uint32_t name_count);

    void (*add_components_watch)(struct ct_comp_watch watch);


    void (*simulate)(struct ct_world world,
                     float dt);

    void (*add_simulation)(uint64_t components_mask,
                           ct_simulate_fce_t simulation);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_WORLD_H
//! \}