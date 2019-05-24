#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component

typedef struct rectangle_component {
    ce_vec2_t half_size;
} rectangle_component;

#define RECTANGLE_COMPONENT \
    CE_ID64_0("rectangle", 0xc2352da254c3b48aULL)

static uint64_t rectangle_cdb_type() {
    return ce_id_a0->id64("rectangle");
}

static const char *rectangle_display_name() {
    return "Rectangle";
}

static uint64_t rectangle_size() {
    return sizeof(rectangle_component);
}

static void _rectangle_on_spawn(uint64_t obj,
                                   void *data) {
    rectangle_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(rectangle_component));
}

static const ce_cdb_prop_def_t0 rectangle_component_prop[] = {
        {
                .name = "half_size",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static struct ct_ecs_component_i0 rectangle_component_i = {
        .display_name = rectangle_display_name,
        .cdb_type = rectangle_cdb_type,
        .size = rectangle_size,
        .on_spawn = _rectangle_on_spawn,
        .on_change = _rectangle_on_spawn,
};
