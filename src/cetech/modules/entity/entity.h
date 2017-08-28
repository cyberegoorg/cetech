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

typedef struct yaml_node_s yaml_node_t;

struct ct_entity_compile_output;
struct ct_compilator_api;

//==============================================================================
// Enums
//==============================================================================

enum ct_property_type {
    PROPERTY_INVALID = 0,
    PROPERTY_BOOL,
    PROPERTY_FLOAT,
    PROPERTY_STRING,
    PROPERTY_STRINGID64,
    PROPERTY_VEC3,
    PROPERTY_QUATF,
};

//==============================================================================
// Typedefs
//==============================================================================

//! World handler
struct ct_world {
    uint32_t h;
};


//! Component compiler
//! \param body Component body yaml
//! \param data Compiled compoent data
typedef int (*ct_component_compiler_t)(yaml_node_t body,
                                       struct ct_blob *data);

//==============================================================================
// Structs
//==============================================================================

//! Entity typedef
struct ct_entity {
    uint32_t h;
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


struct ct_property_value {
    enum ct_property_type type;

    union {
        int b;
        float f;
        const char *str;
        float vec3f[3];
        float quatf[4];
        uint64_t strid_64;
    } value;
};

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

    void (*set_property)(struct ct_world world,
                         struct ct_entity entity,
                         uint64_t key,
                         struct ct_property_value value);

    struct ct_property_value (*get_property)(struct ct_world world,
                                             struct ct_entity entity,
                                             uint64_t key);

    ct_world_callbacks_t world_clb;
} ct_component_clb_null = {};


//==============================================================================
// Api
//==============================================================================

//! Entity system API V0
struct ct_entity_a0 {
    //! Create new entity
    //! \return New entity
    struct ct_entity (*create)();

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


    //! Spawn entity from resource data
    //! \param world World
    //! \param resource Resource data
    void (*spawn_from_resource)(struct ct_world world,
                                void *resource,
                                struct ct_entity **entities,
                                uint32_t *entities_count);

    //! Spawn entity
    //! \param world World
    //! \param name Resource name
    //! \return New entity
    struct ct_entity (*spawn)(struct ct_world world,
                              uint64_t name);


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
                                    yaml_node_t root,
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
                                    struct ct_blob *build);

    //! Resource compile
    //! \param root Root yaml node
    //! \param filename Filename
    //! \param build Build
    //! \param compilator_api Compilator api
    void (*resource_compiler)(yaml_node_t root,
                              const char *filename,
                              struct ct_blob *build,
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
                   yaml_node_t body,
                   struct ct_blob *data);

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

    void (*set_property)(uint64_t type,
                         struct ct_world world,
                         struct ct_entity entity,
                         uint64_t key,
                         struct ct_property_value value);

    struct ct_property_value (*get_property)(uint64_t type,
                                             struct ct_world world,
                                             struct ct_entity entity,
                                             uint64_t key);
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