//! \defgroup Entity
//! Entity system
//! \{
#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/containers/map.h>
#include <celib/yaml/yaml.h>
#include <celib/string/stringid.h>
#include <celib/handler/handlerid.h>
#include <cetech/world/world.h>
#include <cetech/resource/resource.h>


//==============================================================================
// Typedefs
//==============================================================================

//! Entity typedef
typedef struct {
    union {
        handler_t h;
        u32 idx;
    };
} entity_t;

ARRAY_PROTOTYPE(entity_t);
MAP_PROTOTYPE(entity_t);

//==============================================================================
// Api
//==============================================================================

//! Entity system API V0
struct EntitySystemApiV0 {
    //! Create new entity
    //! \return New entity
    entity_t (*entity_manager_create)();

    //! Destoy entity
    //! \param entity Entity to destroy
    void (*entity_manager_destroy)(entity_t entity);

    //! Is entity alive?
    //! \param entity Entity
    //! \return 1 if entity is alive else 0
    int (*entity_manager_alive)(entity_t entity);


    //! Spawn unit from resource data
    //! \param world World
    //! \param resource Resource data
    //! \return Spawne unit array
    ARRAY_T(entity_t) *(*spawn_from_resource)(world_t world,
                                              void *resource);

    //! Spawn unit
    //! \param world World
    //! \param name Resource name
    //! \return New unit
    entity_t (*spawn)(world_t world,
                      stringid64_t name);

    //! Destroy units
    //! \param world World
    //! \param unit Units
    //! \param count Unit count
    void (*destroy)(world_t world,
                    entity_t *unit,
                    u32 count);

    //! Create compiler output structure
    //! \return New compiler output structure
    struct entity_compile_output *(*compiler_create_output)();

    //! Destroy compiler output structure
    //! \param output Compiler output
    void (*compiler_destroy_output)(struct entity_compile_output *output);

    //! Compile unit
    //! \param output Compile output
    //! \param root Yaml root node
    //! \param filename Resource filename
    //! \param compilator_api Compilator api
    void (*compiler_compile_unit)(struct entity_compile_output *output,
                                  yaml_node_t root,
                                  const char *filename,
                                  struct compilator_api *compilator_api);

    //! Get entity counter from output
    //! \param output Compiler output
    //! \return Entity counter
    u32 (*compiler_ent_counter)(struct entity_compile_output *output);

    //! Write output to build
    //! \param output Output
    //! \param build Build
    void (*compiler_write_to_build)(struct entity_compile_output *output,
                                    ARRAY_T(u8) *build);

    //! Resource compile
    //! \param root Root yaml node
    //! \param filename Filename
    //! \param build Build
    //! \param compilator_api Compilator api
    void (*resource_compiler)(yaml_node_t root,
                              const char *filename,
                              ARRAY_T(u8) *build,
                              struct compilator_api *compilator_api);
};

#endif //CETECH_ENTITY_MANAGER_H
//! \}