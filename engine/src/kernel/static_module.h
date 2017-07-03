//==============================================================================
// Static engine system configuration
//==============================================================================
#include <cetech/kernel/module.h>

extern "C" void _init_static_modules(struct api_v0 *api) {
//    LOAD_STATIC_MODULE(api, developsystem);
//    LOAD_STATIC_MODULE(api, consoleserver);
//    LOAD_STATIC_MODULE(api, resourcesystem);

//#ifdef CETECH_CAN_COMPILE
//    LOAD_STATIC_MODULE(api, resourcecompiler);
//#endif

    LOAD_STATIC_MODULE(api, world);
    LOAD_STATIC_MODULE(api, component);
    LOAD_STATIC_MODULE(api, entity);


    LOAD_STATIC_MODULE(api, keyboard);
    LOAD_STATIC_MODULE(api, gamepad);
    LOAD_STATIC_MODULE(api, mouse);

    LOAD_STATIC_MODULE(api, scenegraph);
    LOAD_STATIC_MODULE(api, transform);
    LOAD_STATIC_MODULE(api, level);

    LOAD_STATIC_MODULE(api, renderer);
    LOAD_STATIC_MODULE(api, camera);

    LOAD_STATIC_MODULE(api, luasys);
}

extern "C" void _shutdown_static_modules(struct api_v0 *api) {
    UNLOAD_STATIC_MODULE(api, developsystem);
    UNLOAD_STATIC_MODULE(api, consoleserver);
    UNLOAD_STATIC_MODULE(api, resourcesystem);

#ifdef CETECH_CAN_COMPILE
    UNLOAD_STATIC_MODULE(api, resourcecompiler);
#endif

    UNLOAD_STATIC_MODULE(api, world);
    UNLOAD_STATIC_MODULE(api, component);
    UNLOAD_STATIC_MODULE(api, entity);


    UNLOAD_STATIC_MODULE(api, keyboard);
    UNLOAD_STATIC_MODULE(api, gamepad);
    UNLOAD_STATIC_MODULE(api, mouse);

    UNLOAD_STATIC_MODULE(api, scenegraph);
    UNLOAD_STATIC_MODULE(api, transform);
    UNLOAD_STATIC_MODULE(api, level);

    UNLOAD_STATIC_MODULE(api, renderer);
    UNLOAD_STATIC_MODULE(api, camera);

    UNLOAD_STATIC_MODULE(api, luasys);
}
