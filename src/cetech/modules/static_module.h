//==============================================================================
// Static engine system configuration
//==============================================================================

extern "C" void init_static_modules() {
    //==========================================================================
    // Engine
    //==========================================================================

    CETECH_ADD_STATIC_MODULE(transform);
    CETECH_ADD_STATIC_MODULE(scenegraph);
    CETECH_ADD_STATIC_MODULE(camera);

    CETECH_ADD_STATIC_MODULE(level);

    CETECH_ADD_STATIC_MODULE(renderer);
    CETECH_ADD_STATIC_MODULE(shader);
    CETECH_ADD_STATIC_MODULE(texture);
    CETECH_ADD_STATIC_MODULE(material);
    CETECH_ADD_STATIC_MODULE(scene);
    CETECH_ADD_STATIC_MODULE(mesh_renderer);

    CETECH_ADD_STATIC_MODULE(viewport);

    CETECH_ADD_STATIC_MODULE(luasys);

    CETECH_ADD_STATIC_MODULE(debugui);

    //==========================================================================
    // Playground
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(playground);
    CETECH_ADD_STATIC_MODULE(level_view);
}
