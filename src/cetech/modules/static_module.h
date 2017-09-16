//==============================================================================
// Static engine system configuration
//==============================================================================

extern "C" void init_static_modules() {
    CETECH_ADD_STATIC_MODULE(machine);

    //==========================================================================
    // Renderer
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(renderer);
    CETECH_ADD_STATIC_MODULE(shader);
    CETECH_ADD_STATIC_MODULE(texture);
    CETECH_ADD_STATIC_MODULE(material);

    //==========================================================================
    // Engine
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(application);

    CETECH_ADD_STATIC_MODULE(world);
    CETECH_ADD_STATIC_MODULE(component);
    CETECH_ADD_STATIC_MODULE(entity);
    CETECH_ADD_STATIC_MODULE(keyboard);
    CETECH_ADD_STATIC_MODULE(gamepad);
    CETECH_ADD_STATIC_MODULE(mouse);

    CETECH_ADD_STATIC_MODULE(transform);
    CETECH_ADD_STATIC_MODULE(scenegraph);
    CETECH_ADD_STATIC_MODULE(camera);
    CETECH_ADD_STATIC_MODULE(scene);
    CETECH_ADD_STATIC_MODULE(mesh_renderer);

    CETECH_ADD_STATIC_MODULE(level);

    //==========================================================================
    // Viewport
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(viewport);
    CETECH_ADD_STATIC_MODULE(fullscreen_pass);
    CETECH_ADD_STATIC_MODULE(geometry_pass);

    //==========================================================================
    // DebugUI
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(debugui);

    //==========================================================================
    // Playground
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(cmd_system);
    CETECH_ADD_STATIC_MODULE(playground);
    CETECH_ADD_STATIC_MODULE(property_inspector);
    CETECH_ADD_STATIC_MODULE(asset_browser);
    CETECH_ADD_STATIC_MODULE(asset_property);
    CETECH_ADD_STATIC_MODULE(asset_preview);
    CETECH_ADD_STATIC_MODULE(texture_property);
    CETECH_ADD_STATIC_MODULE(level_inspector);
    CETECH_ADD_STATIC_MODULE(entity_property);
    CETECH_ADD_STATIC_MODULE(transform_property);
    CETECH_ADD_STATIC_MODULE(level_view);
    CETECH_ADD_STATIC_MODULE(entity_preview);
    CETECH_ADD_STATIC_MODULE(level_preview);
    CETECH_ADD_STATIC_MODULE(material_preview);
}
