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

struct ct_property_editor_a0 {
    void (*draw_object)(ce_cdb_t0 db,
                        uint64_t obj,
                        uint64_t context);
};

CE_MODULE(ct_property_editor_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_PROPERTY_INSPECTOR_H
