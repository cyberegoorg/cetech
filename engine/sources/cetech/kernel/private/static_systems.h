//==============================================================================
// Static engine system configuration
//==============================================================================
#include "module.h"

static void _init_static_modules() {
    ADD_STATIC_PLUGIN(entity);
    ADD_STATIC_PLUGIN(component);
    ADD_STATIC_PLUGIN(world);

    ADD_STATIC_PLUGIN(level);

    ADD_STATIC_PLUGIN(keyboard);
    ADD_STATIC_PLUGIN(gamepad);
    ADD_STATIC_PLUGIN(mouse);

    ADD_STATIC_PLUGIN(renderer);

    ADD_STATIC_PLUGIN(camera);
    ADD_STATIC_PLUGIN(scenegraph);
    ADD_STATIC_PLUGIN(transform);
    ADD_STATIC_PLUGIN(mesh);

    ADD_STATIC_PLUGIN(luasys);
}
