#ifndef CETECH_RESOURCE_UI_H
#define CETECH_RESOURCE_UI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

struct ui_vec4_p0 {
    float min_f;
    float max_f;
    bool color;
};

struct ui_vec3_p0 {
    float min_f;
    float max_f;
    bool color;
};

struct ui_float_p0 {
    float min_f;
    float max_f;
};

struct ct_resource_ui_a0 {
    void (*ui_float)(uint64_t obj,
                     uint64_t property,
                     const char *label,
                     struct ui_float_p0 params);

    void (*ui_bool)(uint64_t obj,
                    uint64_t property,
                    const char *label);

    void (*ui_str)(uint64_t obj,
                   uint64_t property,
                   const char *label,
                   uint32_t i);

    void (*ui_str_combo)(uint64_t obj,
                         uint64_t property,
                         const char *label,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i);

    void (*ui_resource)(uint64_t obj,
                        uint64_t property,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i);

    void (*ui_vec3)(uint64_t obj,
                    const uint64_t property[3],
                    const char *label,
                    struct ui_vec3_p0 params);

    void (*ui_vec4)(uint64_t obj,
                    const uint64_t property[4],
                    const char *label,
                    struct ui_vec4_p0 params);
};

CE_MODULE(ct_resource_ui_a0);

#endif //CETECH_RESOURCE_UI_H
