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

struct ct_entity_compile_output;
struct ct_compilator_api;
struct ct_blob;

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
                                       char **data);

//==============================================================================
// Structs
//==============================================================================

//! Entity typedef
struct ct_entity {
    uint64_t h;
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
    //! Component destoryer
    //! \param world World where is component
    //! \param ents Destroy component for this entity
    //! \param ent_count Entity count
    void (*destroyer)(struct ct_world world,
                      struct ct_entity *ents,
                      uint32_t ent_count);

    //! Component spawner
    //! \param world World where component live
    //! \param ents Create component for this entities
    //! \param cent Entity map
    //! \param ents_parent Parent map
    //! \param ent_count Entity count
    //! \param data Component data
    void (*spawner)(struct ct_world world,
                    struct ct_entity *ents,
                    uint32_t *cent,
                    uint32_t *ents_parent,
                    uint32_t ent_count,
                    void *data);

    ct_world_callbacks_t world_clb;
} ct_component_clb_null = {};

struct ct_cdb_obj_t;

//==============================================================================
// Api
//==============================================================================

//! Entity system API V0
struct ct_entity_a0 {
    //! Create new entity
    //! \return New entity
    struct ct_entity (*create)();

    struct ct_cdb_obj_t* (*ent_obj)(struct ct_entity entity);

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


    void (*compiler)(const char *filename,
                     char **output,
                     struct ct_compilator_api *compilator_api);

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

    //! Create compiler output structure
    //! \return New compiler output structure
    struct ct_entity_compile_output *(*compiler_create_output)();

    //! Destroy compiler output structure
    //! \param output Compiler output
    void (*compiler_destroy_output)(struct ct_entity_compile_output *output);

    //! Compile entity
    //! \param output Compile output
    //! \param root Yaml root node
    //! \param filename Resource filename
    //! \param compilator_api Compilator api
    void (*compiler_compile_entity)(struct ct_entity_compile_output *output,
                                    uint64_t *root,
                                    uint32_t root_count,
                                    const char *filename,
                                    struct ct_compilator_api *compilator_api);

    //! Get entity counter from output
    //! \param output Compiler output
    //! \return Entity counter
    uint32_t (*compiler_ent_counter)(struct ct_entity_compile_output *output);

    //! Write output to build
    //! \param output Output
    //! \param build Build
    void (*compiler_write_to_build)(struct ct_entity_compile_output *output,
                                    const char *filename,
                                    char **build);

    //! Resource compile
    //! \param root Root yaml node
    //! \param filename Filename
    //! \param build Build
    //! \param compilator_api Compilator api
    void (*resource_compiler)(uint64_t root,
                              const char *filename,
                              char **build,
                              struct ct_compilator_api *compilator_api);
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
                   char **data);

    //! Get component spawn order
    //! \param type Component type
    //! \return Spawn order
    uint32_t (*spawn_order)(uint64_t type);

    //! Register new component type
    //! \param type Component type
    //! \param clb Callbacks
    void (*register_type)(uint64_t type,
                          struct ct_component_clb clb);

    //! Spawn components
    //! \param world World where component live
    //! \param type Component type
    //! \param ents Create component for this entities
    //! \param cent Entity map
    //! \param ents_parent Parent map
    //! \param ent_count Entity count
    //! \param data Component data
    void (*spawn)(struct ct_world world,
                  uint64_t type,
                  struct ct_entity *ent_ids,
                  uint32_t *cent,
                  uint32_t *ents_parent,
                  uint32_t ent_count,
                  void *data);

    //! Destroy components
    //! \param world World where component live
    //! \param ent Destroy component for this entities
    //! \param count Entities count
    void (*destroy)(struct ct_world world,
                    struct ct_entity *ent,
                    uint32_t count);
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