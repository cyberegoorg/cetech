#ifndef CETECH_BGFX_SCENE_RESOURCE_H
#define CETECH_BGFX_SCENE_RESOURCE_H

int scene_init(struct api_v0 *api);

void scene_shutdown();

void scene_submit(uint64_t scene,
                  uint64_t geom_name);

void scene_create_graph(world_t world,
                        entity_t entity,
                        uint64_t scene);

uint64_t scene_get_mesh_node(uint64_t scene,
                                 uint64_t mesh);


#endif //CETECH_BGFX_SCENE_RESOURCE_H
