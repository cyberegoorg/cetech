#ifndef CETECH_BGFX_TEXTURE_RESOURCE_H
#define CETECH_BGFX_TEXTURE_RESOURCE_H

int texture_resource_init();

void texture_resource_shutdown();

bgfx_texture_handle_t texture_resource_get(stringid64_t name);

#endif //CETECH_BGFX_TEXTURE_RESOURCE_H
