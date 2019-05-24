
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>

// component

typedef struct velocity_component {
    ce_vec2_t velocity;
} velocity_component;

#define VELOCITY_COMPONENT \
    CE_ID64_0("velocity", 0xa5d1f45b42234278ULL)

static uint64_t velocity_cdb_type() {
    return ce_id_a0->id64("velocity");
}

static const char *velocity_display_name() {
    return "Velocty";
}

static uint64_t velocity_size() {
    return sizeof(velocity_component);
}

static const ce_cdb_prop_def_t0 velocity_component_prop[] = {
        {
                .name = "velocity",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static void _velocity_on_spawn(uint64_t obj,
                                   void *data) {
    velocity_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(velocity_component));
}

static struct ct_ecs_component_i0 velocity_component_i = {
        .display_name = velocity_display_name,
        .cdb_type = velocity_cdb_type,
        .size = velocity_size,
        .on_spawn = _velocity_on_spawn,
        .on_change = _velocity_on_spawn,
};
