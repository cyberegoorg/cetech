#ifndef CETECH_EDITOR_UI_H
#define CETECH_EDITOR_UI_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>

#include <corelib/module.inl>

//==============================================================================
// Typedefs
//==============================================================================


//==============================================================================
// Api
//==============================================================================
struct ct_editor_ui_a0 {
    void (*ui_float)(uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label,
                     float min_f,
                     float max_f);

    void (*ui_bool)(uint64_t obj,
                     uint64_t prop_key_hash,
                     const char *label);

    void (*ui_str)(uint64_t obj,
                   uint64_t prop_key_hash,
                   const char *label,
                   uint32_t i);

    void (*ui_str_combo)(uint64_t obj,
                         uint64_t prop_key_hash,
                         const char *label,
                         void (*combo_items)(uint64_t obj,
                                             char **items,
                                             uint32_t *items_count),
                         uint32_t i);

    void (*ui_resource)(uint64_t obj,
                        uint64_t prop_key_hash,
                        const char *label,
                        uint64_t resource_type,
                        uint32_t i);

    void (*ui_vec3)(uint64_t obj,
                    uint64_t prop_key_hash,
                    const char *label,
                    float min_f,
                    float max_f);
};


CT_MODULE(ct_editor_ui_a0);

#endif //CETECH_EDITOR_UI_H
