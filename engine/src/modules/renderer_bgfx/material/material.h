#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

namespace material {
    int init(struct ct_api_a0 *api);

    void shutdown();

    struct ct_material create(uint64_t name);

    uint32_t get_texture_count(struct ct_material material);

    void set_texture(struct ct_material material,
                     const char *slot,
                     uint64_t texture);

    void set_vec4f(struct ct_material material,
                   const char *slot,
                   vec4f_t v);

    void set_mat33f(struct ct_material material,
                    const char *slot,
                    mat33f_t v);

    void set_mat44f(struct ct_material material,
                    const char *slot,
                    mat44f_t v);


    void use(struct ct_material material);

    void submit(struct ct_material material);
}

namespace material_compiler {
    int compiler(const char *filename,
                 struct ct_vio *source_vio,
                 struct ct_vio *build_vio,
                 struct ct_compilator_api *compilator_api);
}

#endif //CETECH_MATERIAL_H
