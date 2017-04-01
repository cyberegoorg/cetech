#ifndef CETECH_BGFX_TEXTURE_RESOURCE_H
#define CETECH_BGFX_TEXTURE_RESOURCE_H

int texture_init();

void texture_shutdown();

bgfx_texture_handle_t texture_get(stringid64_t name);

#endif //CETECH_BGFX_TEXTURE_RESOURCE_H
