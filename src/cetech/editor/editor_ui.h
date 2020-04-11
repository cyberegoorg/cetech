#ifndef CETECH_ASSET_UI_H
#define CETECH_ASSET_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


#define CT_ASSET_UI_A0_STR "ct_editor_ui_a0"

#define CT_ASSET_UI_API \
    CE_ID64_0("ct_editor_ui_a0", 0x864e3516a3c025aeULL)

#define CT_LOCKED_OBJ \
    CE_ID64_0("locked_obj", 0xa0bd856b4fcd25aeULL)

typedef struct ui_vec4_p0 {
    float min_f;
    float max_f;
    bool color;
} ui_vec4_p0;

typedef struct ui_vec3_p0 {
    float min_f;
    float max_f;
    bool color;
} ui_vec3_p0;

typedef struct ui_vec2_p0 {
    float min_f;
    float max_f;
} ui_vec2_p0;

typedef struct ui_float_p0 {
    float min_f;
    float max_f;
} ui_float_p0;

typedef struct ui_uint64_p0 {
    uint64_t min_f;
    uint64_t max_f;
} ui_uint64_p0;


struct ct_editor_ui_a0 {
    void (*prop_float)(uint64_t obj,
                       const char *label,
                       uint64_t property,
                       ui_float_p0 params);

    void (*prop_uin64)(uint64_t obj,
                       const char *label,
                       uint64_t property,
                       ui_uint64_p0 params);

    void (*prop_bool)(uint64_t obj,
                      const char *label,
                      uint64_t property);

    void (*prop_str)(uint64_t obj,
                     const char *label,
                     uint64_t property,
                     uint32_t i);

    void (*prop_filename)(uint64_t obj,
                          const char *label,
                          uint64_t property,
                          const char *filter,
                          uint32_t i);

    void (*prop_str_combo)(uint64_t obj,
                           const char *label,
                           uint64_t property,
                           void (*combo_items)(uint64_t,
                                               char **,
                                               uint32_t *),
                           uint32_t i);

    void (*prop_str_combo2)(uint64_t obj,
                            const char *label,
                            uint64_t property,
                            const char *const *items,
                            uint32_t items_count,
                            uint32_t i);

    void (*prop_asset)(uint64_t obj,
                       const char *label,
                       uint64_t property,
                       uint64_t asset_type,
                       uint64_t context,
                       uint32_t i);

    void (*prop_vec2)(uint64_t obj,
                      const char *label,
                      const uint64_t property[2],
                      ui_vec2_p0 params);

    void (*prop_vec3)(uint64_t obj,
                      const char *label,
                      const uint64_t property[3],
                      ui_vec3_p0 params);

    void (*prop_vec4)(uint64_t obj,
                      const char *label,
                      const uint64_t property[4],
                      ui_vec4_p0 params);


    bool (*prop_revert_btn)(uint64_t _obj,
                            const uint64_t *props,
                            uint64_t props_n);

    bool (*asset_select_modal)(uint64_t modal_id,
                               uint64_t id,
                               uint64_t asset_type,
                               uint64_t *selected_asset);

    bool (*ui_prop_header)(const char *name, uint64_t id);

    void (*ui_prop_header_end)(bool open);

    void (*ui_prop_body)(uint64_t id);

    void (*ui_prop_body_end)();

    void (*prop_label)(const char *label,
                       uint64_t obj,
                       const uint64_t *props,
                       uint64_t props_n);

    void (*prop_value_begin)(uint64_t obj,
                             const uint64_t *props,
                             uint64_t props_n);

    void (*prop_value_end)();

    void (*begin_disabled)();

    void (*end_disabled)();
};

CE_MODULE(ct_editor_ui_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_ASSET_UI_H
