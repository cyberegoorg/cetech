#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/yaml_cdb.h>
#include <celib/math/math.h>

// component

static const char *viewport_display_name() {
    return "Viewport";
}


static void _viewport_on_spawn(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    viewport_component *c = data;
    c->viewport = create_viewport();
}

static struct ct_ecs_component_i0 viewport_component_i = {
        .display_name = viewport_display_name,
        .cdb_type = VIEWPORT_COMPONENT,
        .size = sizeof(viewport_component),
        .from_cdb_obj = _viewport_on_spawn,
};


