#ifndef CETECH_EDITOR_UI_H
#define CETECH_EDITOR_UI_H

#include <stddef.h>
#include <stdint.h>

#include <celib/module.inl>

struct ct_resource_id;

struct ct_editor_ui_a0 {
    void (*ui_float)(struct ct_resource_id rid,
                     uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label,
                     float min_f,
                     float max_f);

    void (*ui_bool)(struct ct_resource_id rid,
                    uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label);

    void (*ui_str)(struct ct_resource_id rid,
                   uint64_t obj,
                   uint64_t prop_key_hash,
                   const char *label,
                   uint32_t i);

    void (*ui_str_combo)(struct ct_resource_id rid,
                         uint64_t obj,
                         uint64_t prop_key_hash,
                         const char *label,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i);

    void (*ui_resource)(struct ct_resource_id rid,
                        uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i);

    void (*ui_vec3)(struct ct_resource_id rid,
                    uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f);

    void (*ui_vec4)(struct ct_resource_id rid,
                    uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f);

    void (*ui_color)(struct ct_resource_id rid,
                    uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f);
};


CE_MODULE(ct_editor_ui_a0);

#endif //CETECH_EDITOR_UI_H
