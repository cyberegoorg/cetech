//! \defgroup Entity
//! Entity system
//! \{
#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H


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
struct ct_blob;
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


//! Component compiler
//! \param body Component body yaml
//! \param data Compiled compoent data
typedef int (*ct_component_compiler_t)(const char *filename,
                                       uint64_t *component_key,
                                       uint32_t component_key_count,
                                       struct ct_cdb_writer_t *writer);

//==============================================================================
// Structs
//==============================================================================

//! Entity typedef
struct ct_entity {
    uint64_t h;
};

struct ct_component_id {
    uint64_t id;
};

//! World callbacks
typedef struct {
    //! On world create callback
    //! \param world World
    void (*on_created)(struct ct_world world);

    //! On world destroy callback
    //! \param world World
    void (*on_destroy)(struct ct_world world);

    //! On world update callback
    //! \param world World
    //! \param dt Delta time
    void (*on_update)(struct ct_world world,
                      float dt);
} ct_world_callbacks_t;


//! Component callbacks
static struct ct_component_clb {
    ct_world_callbacks_t world_clb;
} ct_component_clb_null = {};

struct ct_cdb_obj_t;

typedef void (*ct_simulate_fce_t)(struct ct_entity *ent,
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


//! Entity system API V0
struct ct_entity_a0 {
    //! Create new entity
    //! \return New entity
    struct ct_entity (*create)();

    struct ct_cdb_obj_t *(*ent_obj)(struct ct_entity entity);

    //! Destroy entities
    //! \param world World
    //! \param entity entities
    //! \param count entity count
    void (*destroy)(struct ct_world world,
                    struct ct_entity *entity,
                    uint32_t count);

    //! Is entity alive?
    //! \param entity Entity
    //! \return 1 if entity is alive else 0
    int (*alive)(struct ct_entity entity);

    //! Spawn entity
    //! \param world World
    //! \param name Resource name
    //! \return New entity
    struct ct_entity (*spawn)(struct ct_world world,
                              uint64_t name);

    struct ct_entity (*spawn_level)(struct ct_world world,
                                    uint64_t name);

    struct ct_entity (*find_by_guid)(struct ct_entity root,
                                     uint64_t guid);

    // NEW
    struct ct_component_id (*register_component)(uint64_t component_name);

    uint64_t (*component_mask)(uint64_t component_name);

    bool (*has)(struct ct_entity ent,
                uint64_t *component_name,
                uint32_t name_count);

    void (*add_components)(struct ct_world world,
                           struct ct_entity ent,
                           uint64_t *component_name,
                           uint32_t name_count);

    void (*remove_component)(struct ct_entity ent,
                             uint64_t component_name);

    void (*add_simulation)(uint64_t components_mask,
                           ct_simulate_fce_t simulation);

    void (*add_components_watch)(struct ct_comp_watch watch);

    void (*simulate)(float dt);
};


//! Component system API V0
struct ct_component_a0 {

    //! Register component compiler
    //! \param type Component type
    //! \param compiler Compiler fce
    //! \param spawn_order Spawn order number
    void (*register_compiler)(uint64_t type,
                              ct_component_compiler_t compiler,
                              uint32_t spawn_order);

    //! Compile component
    //! \param type Component type
    //! \param body Component yaml body
    //! \param data Compiled data
    //! \return 1 if compile is ok else 0
    int (*compile)(uint64_t type,
                   const char *filename,
                   uint64_t *component_key,
                   uint32_t component_key_count,
                   struct ct_cdb_writer_t *writer);

    //! Get component spawn order
    //! \param type Component type
    //! \return Spawn order
    uint32_t (*spawn_order)(uint64_t type);

    //! Register new component type
    //! \param type Component type
    //! \param clb Callbacks
    void (*register_type)(uint64_t type,
                          struct ct_component_clb clb);

};

//! World API V0
struct ct_world_a0 {
    //! Register world calbacks
    //! \param clb Callbacks
    void (*register_callback)(ct_world_callbacks_t clb);

    //! Create new world
    //! \return New world
    struct ct_world (*create)();

    //! Destroy world
    //! \param world World
    void (*destroy)(struct ct_world world);

    //! Update world
    //! \param world World
    //! \param dt Delta time
    void (*update)(struct ct_world world,
                   float dt);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ENTITY_MANAGER_H
//! \}