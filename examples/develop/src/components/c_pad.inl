#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component

typedef struct pad_component {
} pad_component;

#define PAD_COMPONENT \
    CE_ID64_0("pad", 0x4d967a4a49a1ee61ULL)

static uint64_t pad_cdb_type() {
    return ce_id_a0->id64("pad");
}

static const char *pad_display_name() {
    return "Pad";
}

static uint64_t pad_size() {
    return sizeof(pad_component);
}

static void _pad_on_spawn(uint64_t obj,
                                   void *data) {
    pad_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(pad_component));
}

static struct ct_ecs_component_i0 pad_component_i = {
        .display_name = pad_display_name,
        .cdb_type = pad_cdb_type,
        .size = pad_size,
        .on_spawn = _pad_on_spawn,
        .on_change = _pad_on_spawn,
};
