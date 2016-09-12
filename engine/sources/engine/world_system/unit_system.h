#ifndef CETECH_UNIT_SYSTEM_H
#define CETECH_UNIT_SYSTEM_H


int unit_init();

void unit_shutdown();

entity_t unit_spawn_from_resource(world_t world,
                                  void *resource);

entity_t unit_spawn(world_t world,
                    stringid64_t name);

#endif //CETECH_UNIT_SYSTEM_H
