#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component

typedef struct ball_component {
} ball_component;

#define BALL_COMPONENT \
    CE_ID64_0("ball", 0xc57998c0dbf5ddd2ULL)

static uint64_t ball_cdb_type() {
    return ce_id_a0->id64("ball");
}

static const char *ball_display_name() {
    return "Ball";
}

static uint64_t ball_size() {
    return sizeof(ball_component);
}

static void _ball_on_spawn(uint64_t obj,
                                   void *data) {
    ball_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(ball_component));
}

static struct ct_ecs_component_i0 ball_component_i = {
        .display_name = ball_display_name,
        .cdb_type = ball_cdb_type,
        .size = ball_size,
        .on_spawn = _ball_on_spawn,
        .on_change = _ball_on_spawn,
};
