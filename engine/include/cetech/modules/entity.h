//! \defgroup Entity
//! Entity system
//! \{
#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H

#include <cetech/modules/blob.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/math_types.h>


typedef struct yaml_node_s yaml_node_t;

struct entity_compile_output;
struct compilator_api;

//==============================================================================
// Enums
//==============================================================================

enum property_type {
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
typedef struct world_s {
    uint32_t h;
} world_t;


//! Component compiler
//! \param body Component body yaml
//! \param data Compiled compoent data
typedef int (*component_compiler_t)(yaml_node_t body,
                                    struct blob_v0 *data);

//! On world create callback
//! \param world World
typedef void (*world_on_created_t)(struct world_s world);

//! On world destroy callback
//! \param world World
typedef void (*world_on_destroy_t)(struct world_s world);

//! On world update callback
//! \param world World
//! \param dt Delta time
typedef void (*world_on_update_t)(struct world_s world,
                                  float dt);

//==============================================================================
// Structs
//==============================================================================

//! Entity typedef
typedef struct entity_s {
    uint32_t h;
} entity_t;


//! World callbacks
typedef struct {
    world_on_created_t on_created;  //!< On create
    world_on_destroy_t on_destroy;  //!< On destroy
    world_on_update_t on_update;    //!< On update
} world_callbacks_t;


struct property_value {
    enum property_type type;
    union {
        int b;
        float f;
        const char *str;
        vec3f_t vec3f;
        quatf_t quatf;
        uint64_t strid_64;
    } value;
};

//! Component callbacks
static struct component_clb {


    //! Component destoryer
    //! \param world World where is component
    //! \param ents Destroy component for this entity
    //! \param ent_count Entity count
    void (*destroyer)(world_t world,
                      entity_t *ents,
                      size_t ent_count);

    //! Component spawner
    //! \param world World where component live
    //! \param ents Create component for this entities
    //! \param cent Entity map
    //! \param ents_parent Parent map
    //! \param ent_count Entity count
    //! \param data Component data
    void (*spawner)(world_t world,
                    entity_t *ents,
                    uint32_t *cent,
                    uint32_t *ents_parent,
                    size_t ent_count,
                    void *data);

    void (*set_property)(world_t world,
                         entity_t entity,
                         uint64_t key,
                         struct property_value value);

    struct property_value (*get_property)(world_t world,
                                          entity_t entity,
                                          uint64_t key);

    world_on_created_t on_world_create;  //!< On world create
    world_on_destroy_t on_world_destroy; //!< On world destroy
    world_on_update_t on_world_update;   //!< On world update
} component_clb_null = {0};


//==============================================================================
// Api
//==============================================================================

//! Entity system API V0
struct entity_api_v0 {
    //! Create new entity
    //! \return New entity
    entity_t (*create)();

    //! Destroy entities
    //! \param world World
    //! \param entity entities
    //! \param count entity count
    void (*destroy)(world_t world,
                    entity_t *entity,
                    uint32_t count);

    //! Is entity alive?
    //! \param entity Entity
    //! \return 1 if entity is alive else 0
    int (*alive)(entity_t entity);


    //! Spawn entity from resource data
    //! \param world World
    //! \param resource Resource data
    void (*spawn_from_resource)(world_t world,
                                void *resource,
                                entity_t **entities,
                                uint32_t *entities_count);

    //! Spawn entity
    //! \param world World
    //! \param name Resource name
    //! \return New entity
    entity_t (*spawn)(world_t world,
                      uint64_t name);

#ifdef CETECH_CAN_COMPILE

    //! Create compiler output structure
    //! \return New compiler output structure
    struct entity_compile_output *(*compiler_create_output)();

    //! Destroy compiler output structure
    //! \param output Compiler output
    void (*compiler_destroy_output)(struct entity_compile_output *output);

    //! Compile entity
    //! \param output Compile output
    //! \param root Yaml root node
    //! \param filename Resource filename
    //! \param compilator_api Compilator api
    void (*compiler_compile_entity)(struct entity_compile_output *output,
                                    yaml_node_t root,
                                    const char *filename,
                                    struct compilator_api *compilator_api);

    //! Get entity counter from output
    //! \param output Compiler output
    //! \return Entity counter
    uint32_t (*compiler_ent_counter)(struct entity_compile_output *output);

    //! Write output to build
    //! \param output Output
    //! \param build Build
    void (*compiler_write_to_build)(struct entity_compile_output *output,
                                    struct blob_v0 *build);

    //! Resource compile
    //! \param root Root yaml node
    //! \param filename Filename
    //! \param build Build
    //! \param compilator_api Compilator api
    void (*resource_compiler)(yaml_node_t root,
                              const char *filename,
                              struct blob_v0 *build,
                              struct compilator_api *compilator_api);

#endif
};


//! Component system API V0
struct component_api_v0 {
    //! Register component compiler
    //! \param type Component type
    //! \param compiler Compiler fce
    //! \param spawn_order Spawn order number
    void (*register_compiler)(uint64_t type,
                              component_compiler_t compiler,
                              uint32_t spawn_order);

    //! Compile component
    //! \param type Component type
    //! \param body Component yaml body
    //! \param data Compiled data
    //! \return 1 if compile is ok else 0
    int (*compile)(uint64_t type,
                   yaml_node_t body,
                   struct blob_v0 *data);

    //! Get component spawn order
    //! \param type Component type
    //! \return Spawn order
    uint32_t (*spawn_order)(uint64_t type);

    //! Register new component type
    //! \param type Component type
    //! \param clb Callbacks
    void (*register_type)(uint64_t type,
                          struct component_clb clb);

    //! Spawn components
    //! \param world World where component live
    //! \param type Component type
    //! \param ents Create component for this entities
    //! \param cent Entity map
    //! \param ents_parent Parent map
    //! \param ent_count Entity count
    //! \param data Component data
    void (*spawn)(world_t world,
                  uint64_t type,
                  entity_t *ent_ids,
                  uint32_t *cent,
                  uint32_t *ents_parent,
                  uint32_t ent_count,
                  void *data);

    //! Destroy components
    //! \param world World where component live
    //! \param ent Destroy component for this entities
    //! \param count Entities count
    void (*destroy)(world_t world,
                    entity_t *ent,
                    uint32_t count);

    void (*set_property)(uint64_t type,
                         world_t world,
                         entity_t entity,
                         uint64_t key,
                         struct property_value value);

    struct property_value (*get_property)(uint64_t type,
                                          world_t world,
                                          entity_t entity,
                                          uint64_t key);
};

//! World API V0
struct world_api_v0 {

    //! Register world calbacks
    //! \param clb Callbacks
    void (*register_callback)(world_callbacks_t clb);

    //! Create new world
    //! \return New world
    world_t (*create)();

    //! Destroy world
    //! \param world World
    void (*destroy)(world_t world);

    //! Update world
    //! \param world World
    //! \param dt Delta time
    void (*update)(world_t world,
                   float dt);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ENTITY_MANAGER_H
//! \}