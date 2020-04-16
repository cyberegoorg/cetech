#ifndef CETECH_PROPERTY_INSPECTOR_H
#define CETECH_PROPERTY_INSPECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define CT_PROP_EDITOR_A0_STR "ct_property_editor_a0"

#define CT_PROP_EDITOR_API \
    CE_ID64_0("ct_property_editor_a0", 0x10996d5471937b8aULL)

#define CT_PROPERTY_EDITOR_I0 \
    CE_ID64_0("ct_property_editor_i0", 0x50566f03b5eacb95ULL)


#define CT_PROPERTY_EDITOR_ASPECT \
    CE_ID64_0("ct_property_editor_aspect", 0x5e2c5567004ac231ULL)

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


struct ct_property_editor_a0 {
    void (*draw_object)(ce_cdb_t0 db,
                        uint64_t obj,
                        const char* title,
                        uint64_t context);

    void (*ui_float)(uint64_t obj,
                     const char *label,
                     uint64_t property,
                     ui_float_p0 params);

    void (*ui_uin64)(uint64_t obj,
                     const char *label,
                     uint64_t property,
                     ui_uint64_p0 params);

    void (*ui_bool)(uint64_t obj,
                    const char *label,
                    uint64_t property);

    void (*ui_str)(uint64_t obj,
                   const char *label,
                   uint64_t property,
                   uint32_t i);

    void (*ui_filename)(uint64_t obj,
                        const char *label,
                        uint64_t property,
                        const char *filter,
                        uint32_t i);

    void (*ui_str_combo)(uint64_t obj,
                         const char *label,
                         uint64_t property,
                         void (*combo_items)(uint64_t,
                                             char **,
                                             uint32_t *),
                         uint32_t i);

    void (*ui_str_combo2)(uint64_t obj,
                          const char *label,
                          uint64_t property,
                          const char *const *items,
                          uint32_t items_count,
                          uint32_t i);

    void (*ui_asset)(uint64_t obj,
                     const char *label,
                     uint64_t property,
                     uint64_t asset_type,
                     uint64_t context,
                     uint32_t i);

    void (*ui_vec2)(uint64_t obj,
                    const char *label,
                    const uint64_t property[2],
                    ui_vec2_p0 params);

    void (*ui_vec3)(uint64_t obj,
                    const char *label,
                    const uint64_t property[3],
                    ui_vec3_p0 params);

    void (*ui_vec4)(uint64_t obj,
                    const char *label,
                    const uint64_t property[4],
                    ui_vec4_p0 params);


    bool (*ui_revert_btn)(uint64_t _obj,
                          const uint64_t *props,
                          uint64_t props_n);

    bool (*ui_asset_select_modal)(uint64_t modal_id,
                                  uint64_t id,
                                  uint64_t asset_type,
                                  uint64_t *selected_asset);

    bool (*ui_header_begin)(const char *name,
                            uint64_t id);

    void (*ui_header_end)(bool open,
                          uint64_t id);

    void (*ui_body_begin)(uint64_t id);

    void (*ui_body_end)(uint64_t id);

    void (*ui_label)(const char *label,
                     uint64_t obj,
                     const uint64_t *props,
                     uint64_t props_n);

    void (*ui_value_begin)(uint64_t obj,
                           const uint64_t *props,
                           uint64_t props_n);

    void (*ui_value_end)();

    void (*ui_disabled_begin)();

    void (*ui_disabled_end)();
};

CE_MODULE(ct_property_editor_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_PROPERTY_INSPECTOR_H
