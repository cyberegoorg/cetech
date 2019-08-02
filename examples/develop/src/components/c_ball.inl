#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/yaml_cdb.h>
#include <celib/math/math.h>

// component
#define BALL_COMPONENT \
    CE_ID64_0("ball", 0xc57998c0dbf5ddd2ULL)

static const char *ball_display_name() {
    return "Ball";
}

static struct ct_ecs_component_i0 ball_component_i = {
        .display_name = ball_display_name,
        .cdb_type = BALL_COMPONENT,
};
