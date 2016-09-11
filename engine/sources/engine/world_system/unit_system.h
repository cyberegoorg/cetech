#ifndef CETECH_UNIT_SYSTEM_H
#define CETECH_UNIT_SYSTEM_H


int unit_init();

void unit_shutdown();

entity_t unit_spawn_from_resource(world_t world,
                                  void *resource);

#endif //CETECH_UNIT_SYSTEM_H
