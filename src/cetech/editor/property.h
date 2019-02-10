#ifndef CETECH_PROPERTY_INSPECTOR_H
#define CETECH_PROPERTY_INSPECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define CT_PROP_EDITOR_API \
    CE_ID64_0("ct_property_editor_a0", 0x10996d5471937b8aULL)


#define PROPERTY_EDITOR_INTERFACE \
    CE_ID64_0("ct_property_editor_i0", 0x50566f03b5eacb95ULL)

typedef struct ct_resource_id_t0 ct_resource_id_t0;

typedef struct ct_property_editor_i0 {
    uint64_t (*cdb_type)();

    void (*draw_ui)(uint64_t obj, uint64_t context);

    void (*draw_menu)(uint64_t obj);
}ct_property_editor_i0;


struct ct_property_editor_a0 {
    void (*draw)(uint64_t obj, uint64_t context);

    ct_property_editor_i0 *(*get_interface)(uint64_t obj);
};

CE_MODULE(ct_property_editor_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_PROPERTY_INSPECTOR_H
