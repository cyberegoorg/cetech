#ifndef CETECH_BGFX_SHADER_RESOURCE_H
#define CETECH_BGFX_SHADER_RESOURCE_H

#include <bgfx/c99/bgfx.h>

int shader_init(struct api_v0 *api);

void shader_shutdown();

bgfx_program_handle_t shader_get(uint64_t name);

#endif //CETECH_BGFX_SHADER_RESOURCE_H
