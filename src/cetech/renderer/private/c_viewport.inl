#include <stdlib.h>

#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

// component

static uint64_t viewport_cdb_type() {
    return ce_id_a0->id64("viewport");
}

static const char *viewport_display_name() {
    return "Viewport";
}

static uint64_t viewport_component_size() {
    return sizeof(viewport_component);
}

static void _viewport_on_spawn(uint64_t obj,
                                   void *data) {
    viewport_component *c = data;
    c->viewport = create_viewport();
}

static struct ct_ecs_component_i0 viewport_component_i = {
        .display_name = viewport_display_name,
        .cdb_type = viewport_cdb_type,
        .size = viewport_component_size,
        .on_spawn = _viewport_on_spawn,
        .on_change = _viewport_on_spawn,
};


