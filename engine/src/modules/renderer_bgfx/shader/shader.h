#ifndef CETECH_BGFX_SHADER_RESOURCE_H
#define CETECH_BGFX_SHADER_RESOURCE_H


namespace shader {
    int shader_init(struct ct_api_a0 *api);

    void shader_shutdown();

    bgfx::ProgramHandle shader_get(uint64_t name);
}

#endif //CETECH_BGFX_SHADER_RESOURCE_H
