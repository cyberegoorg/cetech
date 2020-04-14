#include <stdlib.h>

#include <cetech/property_editor/property_editor.h>
#include <celib/yaml_cdb.h>
#include <celib/math/math.h>

// component
#define BALL_COMPONENT \
    CE_ID64_0("ball", 0xc57998c0dbf5ddd2ULL)

static struct ct_ecs_component_i0 ball_component_i = {
        .display_name = "Ball",
        .cdb_type = BALL_COMPONENT,
};
