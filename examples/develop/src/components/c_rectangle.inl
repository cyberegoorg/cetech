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

#define RECTANGLE_HALF_SIZE_PROP \
    CE_ID64_0("half_size", 0xfae83ca4619fb60aULL)

static const char *rectangle_display_name() {
    return "Rectangle";
}

static void _rectangle_on_spawn(ct_world_t0 world,
                                ce_cdb_t0 db,
                                uint64_t obj,
                                void *data) {
    rectangle_component *c = data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t vec = ce_cdb_a0->read_subobject(r, RECTANGLE_HALF_SIZE_PROP, 0);
    const ce_cdb_obj_o0 *vec_r = ce_cdb_a0->read(db, vec);

    *c = (rectangle_component){
        .half_size = (ce_vec2_t) {
                .x = ce_cdb_a0->read_float(vec_r, PROP_VEC_X, 0.0f),
                .y = ce_cdb_a0->read_float(vec_r, PROP_VEC_Y, 0.0f),
        }
    };
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
        .cdb_type = RECTANGLE_COMPONENT,
        .size = sizeof(rectangle_component),
        .from_cdb_obj = _rectangle_on_spawn,
};
