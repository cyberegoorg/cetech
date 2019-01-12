#ifndef CETECH_RESOURCE_UI_H
#define CETECH_RESOURCE_UI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define CT_RESOURCE_UI_API \
    CE_ID64_0("ct_editor_ui_a0", 0x864e3516a3c025aeULL)


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

struct ct_editor_ui_a0 {
    void (*prop_float)(uint64_t obj,
                       uint64_t property,
                       const char *label,
                       struct ui_float_p0 params);

    void (*prop_bool)(uint64_t obj,
                      uint64_t property,
                      const char *label);

    void (*prop_str)(uint64_t obj,
                     uint64_t property,
                     const char *label,
                     uint32_t i);

    void (*prop_str_combo)(uint64_t obj,
                           uint64_t property,
                           const char *label,
                           void (*combo_items)(uint64_t obj,
                                               char **items,
                                               uint32_t *items_count),
                           uint32_t i);

    void (*prop_resource)(uint64_t obj,
                          uint64_t property,
                          const char *label,
                          uint64_t resource_type,
                          uint32_t i);

    void (*prop_vec3)(uint64_t obj,
                      const uint64_t property[3],
                      const char *label,
                      struct ui_vec3_p0 params);

    void (*prop_vec4)(uint64_t obj,
                      const uint64_t property[4],
                      const char *label,
                      struct ui_vec4_p0 params);


    bool (*prop_revert_btn)(uint64_t _obj,
                            const uint64_t *props,
                            uint64_t props_n);

    void (*resource_tooltip)(struct ct_resource_id resourceid,
                             const char *path);

    bool (*resource_select_modal)(const char *modal_id,
                                  uint64_t id,
                                  uint64_t resource_type,
                                  uint64_t *selected_resource);
};

CE_MODULE(ct_editor_ui_a0);

#endif //CETECH_RESOURCE_UI_H
