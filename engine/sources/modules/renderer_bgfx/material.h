#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

namespace material {
    int init(struct api_v0 *api);
    void shutdown();

    material_t create(uint64_t name);

    uint32_t get_texture_count(material_t material);

    void set_texture(material_t material,
                     const char *slot,
                     uint64_t texture);

    void set_vec4f(material_t material,
                   const char *slot,
                   vec4f_t v);

    void set_mat33f(material_t material,
                    const char *slot,
                    mat33f_t v);

    void set_mat44f(material_t material,
                    const char *slot,
                    mat44f_t v);


    void use(material_t material);

    void submit(material_t material);
}

namespace material_resource_compiler {
    int compiler(const char *filename,
                 struct vio *source_vio,
                 struct vio *build_vio,
                 struct compilator_api *compilator_api);
}

#endif //CETECH_MATERIAL_H
