#ifndef CETECH__MATERIAL_H
#define CETECH__MATERIAL_H

enum material_variable_type {
    MAT_VAR_NONE = 0,
    MAT_VAR_INT,
    MAT_VAR_TEXTURE,
    MAT_VAR_TEXTURE_HANDLER, //TODO: RENAME
    MAT_VAR_VEC4,
    MAT_VAR_MAT44
};

struct material_variable {
    material_variable_type type;
    union {
        int i;
        float f;
        uint64_t t;
        uint16_t th;
        float v3[3];
        float v4[4];
        float m44[16];
    };
};

#endif //CETECH__MATERIAL_H
