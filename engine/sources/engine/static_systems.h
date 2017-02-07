//==============================================================================
// Static engine system configuration
//==============================================================================
#include "engine/plugin/plugin.h"

void _init_static_plugins() {
    ADD_STATIC_PLUGIN(config);
    ADD_STATIC_PLUGIN(task);

    ADD_STATIC_PLUGIN(filesystem);
    ADD_STATIC_PLUGIN(resourcecompiler);

    ADD_STATIC_PLUGIN(consoleserver);
    ADD_STATIC_PLUGIN(developsystem);

    ADD_STATIC_PLUGIN(keyboard);
    ADD_STATIC_PLUGIN(gamepad);
    ADD_STATIC_PLUGIN(mouse);

    ADD_STATIC_PLUGIN(resourcesystem);
    ADD_STATIC_PLUGIN(entcom);
    ADD_STATIC_PLUGIN(world);

    ADD_STATIC_PLUGIN(unit);
    ADD_STATIC_PLUGIN(level);

    ADD_STATIC_PLUGIN(renderer);
    ADD_STATIC_PLUGIN(luasys);

    ADD_STATIC_PLUGIN(camera);
    ADD_STATIC_PLUGIN(scenegraph);
    ADD_STATIC_PLUGIN(transform);
    ADD_STATIC_PLUGIN(mesh);
}

