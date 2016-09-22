#ifndef CETECH_UNIT_SYSTEM_H
#define CETECH_UNIT_SYSTEM_H

#include "../develop/resource_compiler.h"

int unit_init(int stage);

void unit_shutdown();

entity_t unit_spawn_from_resource(world_t world,
                                  void *resource);

entity_t unit_spawn(world_t world,
                    stringid64_t name);

void unit_resource_compiler(yaml_node_t root,
                            const char *filename,
                            ARRAY_T(u8) *build,
                            struct compilator_api *compilator_api);

#endif //CETECH_UNIT_SYSTEM_H
