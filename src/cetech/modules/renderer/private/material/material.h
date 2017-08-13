#ifndef CETECH_MATERIAL_H
#define CETECH_MATERIAL_H

enum material_variable_type {
    MAT_VAR_NONE = 0,
    MAT_VAR_INT,
    MAT_VAR_TEXTURE,
    MAT_VAR_VEC4,
    MAT_VAR_MAT44
};

struct material_variable {
    material_variable_type type;
    union {
        int i;
        float f;
        uint64_t t;
        float v3[3];
        float v4[4];
        float m44[16];
    };
};

namespace material {
    int init(struct ct_api_a0 *api);

    void shutdown();

    struct ct_material create(uint64_t name);

    void set_texture(struct ct_material material,
                     const char *slot,
                     uint64_t texture);


    void set_mat44f(struct ct_material material,
                    const char *slot,
                    float *value);


    void use(struct ct_material material);

    void submit(struct ct_material material,  uint64_t layer, uint8_t viewid);
}

namespace material_compiler {
    int compiler(const char *filename,
                 struct ct_vio *source_vio,
                 struct ct_vio *build_vio,
                 struct ct_compilator_api *compilator_api);
}

#endif //CETECH_MATERIAL_H
