//==============================================================================
// Static engine system configuration
//==============================================================================

extern "C" void init_static_modules() {
    CETECH_ADD_STATIC_MODULE(transform);
    CETECH_ADD_STATIC_MODULE(scenegraph);
    CETECH_ADD_STATIC_MODULE(camera);

    CETECH_ADD_STATIC_MODULE(level);
    CETECH_ADD_STATIC_MODULE(renderer);
    CETECH_ADD_STATIC_MODULE(luasys);

    CETECH_ADD_STATIC_MODULE(debugui);
    CETECH_ADD_STATIC_MODULE(playground);
}
