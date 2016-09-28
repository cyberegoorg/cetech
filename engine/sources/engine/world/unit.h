#ifndef CETECH_UNIT_SYSTEM_H
#define CETECH_UNIT_SYSTEM_H

#include "engine/develop/resource_compiler.h"


int unit_init(int stage);

void unit_shutdown();

entity_t *unit_spawn_from_resource(world_t world,
                                   void *resource,
                                   ARRAY_T(entity_t) *spawned);

entity_t unit_spawn(world_t world,
                    stringid64_t name);

void unit_destroy(world_t world,
                  entity_t *unit,
                  u32 count);

struct entity_compile_output *unit_compiler_create_output();

void unit_compiler_destroy_output(struct entity_compile_output *output);

void unit_compiler_compile_unit(struct entity_compile_output *output,
                                yaml_node_t root,
                                const char *filename,
                                struct compilator_api *compilator_api);

u32 unit_compiler_ent_counter(struct entity_compile_output *output);

void unit_compiler_write_to_build(struct entity_compile_output *output,
                                  ARRAY_T(u8) *build);

void unit_resource_compiler(yaml_node_t root,
                            const char *filename,
                            ARRAY_T(u8) *build,
                            struct compilator_api *compilator_api);

#endif //CETECH_UNIT_SYSTEM_H
