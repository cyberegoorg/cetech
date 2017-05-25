//! \defgroup Component
//! Component system
//! \{
#ifndef CETECH_COMPONENT_MANAGER_H
#define CETECH_COMPONENT_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/math_types.h>

struct stringid64_s;

typedef struct world_s world_t;
typedef struct entity_s entity_t;
typedef struct stringid64_s stringid64_t;
typedef struct yaml_node_s yaml_node_t;

enum property_type {
    PROPERTY_INVALID = 0,
    PROPERTY_BOOL,
    PROPERTY_FLOAT,
    PROPERTY_STRING,
    PROPERTY_STRINGID64,
    PROPERTY_VEC3,
    PROPERTY_QUATF,
};

struct property_value {
    enum property_type type;
    union {
        int b;
        float f;
        const char *str;
        vec3f_t vec3f;
        quatf_t quatf;
        stringid64_t strid_64;
    } value;
};

enum {
    COMPONENT_API_ID = 12
};

//==============================================================================
// Typedefs
//==============================================================================


//! Component compiler
//! \param body Component body yaml
//! \param data Compiled compoent data
typedef int (*component_compiler_t)(yaml_node_t body,
                                    ARRAY_T(uint8_t) *data);

//! Component destoryer
//! \param world World where is component
//! \param ents Destroy component for this entity
//! \param ent_count Entity count
typedef void (*component_destroyer_t)(world_t world,
                                      entity_t *ents,
                                      size_t ent_count);

//! Component spawner
//! \param world World where component live
//! \param ents Create component for this entities
//! \param cent Entity map
//! \param ents_parent Parent map
//! \param ent_count Entity count
//! \param data Component data
typedef void (*component_spawner_t)(world_t world,
                                    entity_t *ents,
                                    uint32_t *cent,
                                    uint32_t *ents_parent,
                                    size_t ent_count,
                                    void *data);

typedef void (*component_set_property_t)(world_t world,
                                         entity_t entity,
                                         stringid64_t key,
                                         struct property_value value);

typedef struct property_value (*component_get_property_t)(world_t world,
                                                          entity_t entity,
                                                          stringid64_t key);

//==============================================================================
// Structs
//==============================================================================

//! Component callbacks
static struct component_clb {
    component_spawner_t spawner;         //!< Spawner
    component_destroyer_t destroyer;     //!< Destroyer
    world_on_created_t on_world_create;  //!< On world create
    world_on_destroy_t on_world_destroy; //!< On world destroy
    world_on_update_t on_world_update;   //!< On world update
    component_set_property_t set_property;
    component_get_property_t get_property;
} component_clb_null = {0};

//==============================================================================
// Api
//==============================================================================

//! Component system API V0
struct component_api_v0 {
    //! Register component compiler
    //! \param type Component type
    //! \param compiler Compiler fce
    //! \param spawn_order Spawn order number
    void (*component_register_compiler)(stringid64_t type,
                                        component_compiler_t compiler,
                                        uint32_t spawn_order);

    //! Compile component
    //! \param type Component type
    //! \param body Component yaml body
    //! \param data Compiled data
    //! \return 1 if compile is ok else 0
    int (*component_compile)(stringid64_t type,
                             yaml_node_t body,
                             ARRAY_T(uint8_t) *data);

    //! Get component spawn order
    //! \param type Component type
    //! \return Spawn order
    uint32_t (*component_get_spawn_order)(stringid64_t type);

    //! Register new component type
    //! \param type Component type
    //! \param clb Callbacks
    void (*component_register_type)(stringid64_t type,
                                    struct component_clb clb);

    //! Spawn components
    //! \param world World where component live
    //! \param type Component type
    //! \param ents Create component for this entities
    //! \param cent Entity map
    //! \param ents_parent Parent map
    //! \param ent_count Entity count
    //! \param data Component data
    void (*component_spawn)(world_t world,
                            stringid64_t type,
                            entity_t *ent_ids,
                            uint32_t *cent,
                            uint32_t *ents_parent,
                            uint32_t ent_count,
                            void *data);

    //! Destroy components
    //! \param world World where component live
    //! \param ent Destroy component for this entities
    //! \param count Entities count
    void (*component_destroy)(world_t world,
                              entity_t *ent,
                              uint32_t count);

    void (*set_property)(stringid64_t type,
                         world_t world,
                         entity_t entity,
                         stringid64_t key,
                         struct property_value value);

    struct property_value (*get_property)(stringid64_t type,
                                          world_t world,
                                          entity_t entity,
                                          stringid64_t key);
};

#endif //CETECH_COMPONENT_MANAGER_H
//! \}