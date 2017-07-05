//==============================================================================
// Static engine system configuration
//==============================================================================
#include <cetech/kernel/module.h>

extern "C" void init_static_modules() {
    ADD_STATIC_MODULE(blob);
    ADD_STATIC_MODULE(filesystem);
    ADD_STATIC_MODULE(resourcesystem);

#ifdef CETECH_CAN_COMPILE
    ADD_STATIC_MODULE(resourcecompiler);
#endif



    ADD_STATIC_MODULE(world);
    ADD_STATIC_MODULE(component);
    ADD_STATIC_MODULE(entity);
    ADD_STATIC_MODULE(keyboard);
    ADD_STATIC_MODULE(gamepad);
    ADD_STATIC_MODULE(mouse);
    ADD_STATIC_MODULE(scenegraph);
    ADD_STATIC_MODULE(transform);
    ADD_STATIC_MODULE(level);
    ADD_STATIC_MODULE(renderer);
    ADD_STATIC_MODULE(camera);
    ADD_STATIC_MODULE(luasys);
}
