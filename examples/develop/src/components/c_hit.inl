#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component

typedef struct hit_component {
    ct_entity_t0 ent2;
    ce_vec2_t normal;
} hit_component;

#define HIT_COMPONENT \
    CE_ID64_0("hit", 0x6f2598e77d07074cULL)

static uint64_t hit_cdb_type() {
    return ce_id_a0->id64("hit");
}

static const char *hit_display_name() {
    return "Hit";
}

static uint64_t hit_size() {
    return sizeof(hit_component);
}

static void _hit_on_spawn(uint64_t obj,
                                   void *data) {
    hit_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(hit_component));
}

static struct ct_ecs_component_i0 hit_component_i = {
        .display_name = hit_display_name,
        .cdb_type = hit_cdb_type,
        .size = hit_size,
        .on_spawn = _hit_on_spawn,
        .on_change = _hit_on_spawn,
};
