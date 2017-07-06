#ifndef CETECH_BGFX_SCENE_RESOURCE_H
#define CETECH_BGFX_SCENE_RESOURCE_H

namespace scene {
    int init(struct ct_api_a0 *api);

    void shutdown();

    void submit(uint64_t scene,
                uint64_t geom_name);

    void create_graph(struct ct_world world,
                      struct ct_entity entity,
                      uint64_t scene);

    uint64_t get_mesh_node(uint64_t scene,
                           uint64_t mesh);
}

#endif //CETECH_BGFX_SCENE_RESOURCE_H
