//! \addtogroup World
//! \{

#ifndef CETECH_UNIT_SYSTEM_H
#define CETECH_UNIT_SYSTEM_H

#include <celib/containers/array.h>
#include <engine/entcom/entcom.h>
#include <celib/yaml/yaml.h>
#include "engine/resource/types.h"
#include "engine/world/world.h"

//! Unit API V!
struct UnitApiV0 {

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


#endif //CETECH_UNIT_SYSTEM_H
//! \}