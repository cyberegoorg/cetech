#ifndef CETECH_BGFX_SHADER_RESOURCE_H
#define CETECH_BGFX_SHADER_RESOURCE_H

#include <bgfx/c99/bgfx.h>

int shader_resource_init();

void shader_resource_shutdown();

bgfx_program_handle_t shader_resource_get(stringid64_t name);

#endif //CETECH_BGFX_SHADER_RESOURCE_H
