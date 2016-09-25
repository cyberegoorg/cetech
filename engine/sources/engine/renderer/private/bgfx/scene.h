#ifndef CETECH_BGFX_SCENE_RESOURCE_H
#define CETECH_BGFX_SCENE_RESOURCE_H

int scene_resource_init();

void scene_resource_shutdown();

void scene_resource_submit(stringid64_t scene,
                           stringid64_t geom_name);

#endif //CETECH_BGFX_SCENE_RESOURCE_H
