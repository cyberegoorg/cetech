#ifndef CETECH_BGFX_SCENE_RESOURCE_H
#define CETECH_BGFX_SCENE_RESOURCE_H

int scene_resource_init();

void scene_resource_shutdown();

void scene_resource_submit(stringid64_t scene,
                           stringid64_t geom_name);

void scene_create_graph(world_t world,
                        entity_t entity,
                        stringid64_t scene);

stringid64_t scene_get_mesh_node(stringid64_t scene,
                                 stringid64_t mesh);


#endif //CETECH_BGFX_SCENE_RESOURCE_H
