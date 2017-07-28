
//==============================================================================
// Static engine system configuration
//==============================================================================
#include <cetech/core/module.h>

extern "C" void init_static_modules() {
    CETECH_ADD_STATIC_MODULE(blob);
    CETECH_ADD_STATIC_MODULE(machine);

    CETECH_ADD_STATIC_MODULE(developsystem);
    CETECH_ADD_STATIC_MODULE(task);
    CETECH_ADD_STATIC_MODULE(consoleserver);

    CETECH_ADD_STATIC_MODULE(filesystem);
    CETECH_ADD_STATIC_MODULE(resourcesystem);

#ifdef CETECH_CAN_COMPILE
    CETECH_ADD_STATIC_MODULE(resourcecompiler);
#endif

    CETECH_ADD_STATIC_MODULE(world);
    CETECH_ADD_STATIC_MODULE(component);
    CETECH_ADD_STATIC_MODULE(entity);
    CETECH_ADD_STATIC_MODULE(keyboard);
    CETECH_ADD_STATIC_MODULE(gamepad);
    CETECH_ADD_STATIC_MODULE(mouse);
    CETECH_ADD_STATIC_MODULE(scenegraph);
    CETECH_ADD_STATIC_MODULE(transform);
    CETECH_ADD_STATIC_MODULE(level);
    CETECH_ADD_STATIC_MODULE(renderer);
    CETECH_ADD_STATIC_MODULE(camera);
    CETECH_ADD_STATIC_MODULE(luasys);
}
