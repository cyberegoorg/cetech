//==============================================================================
// Static engine system configuration
//==============================================================================

void init_static_modules() {
    CETECH_ADD_STATIC_MODULE(machine);
    CETECH_ADD_STATIC_MODULE(ecs);

    //==========================================================================
    // Renderer
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(renderer);
    CETECH_ADD_STATIC_MODULE(shader);
    CETECH_ADD_STATIC_MODULE(texture);
    CETECH_ADD_STATIC_MODULE(material);
    CETECH_ADD_STATIC_MODULE(debugdraw);
    CETECH_ADD_STATIC_MODULE(render_graph);

    //==========================================================================
    // Engine
    //==========================================================================


    CETECH_ADD_STATIC_MODULE(keyboard);
    CETECH_ADD_STATIC_MODULE(gamepad);
    CETECH_ADD_STATIC_MODULE(mouse);


    //==========================================================================
    // DebugUI
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(debugui);

    //==========================================================================
    // Playground
    //==========================================================================
    CETECH_ADD_STATIC_MODULE(cmd_system);
    CETECH_ADD_STATIC_MODULE(action_manager);

    CETECH_ADD_STATIC_MODULE(playground);
    CETECH_ADD_STATIC_MODULE(asset_browser);
    CETECH_ADD_STATIC_MODULE(level_inspector);
    CETECH_ADD_STATIC_MODULE(entity_property);

    CETECH_ADD_STATIC_MODULE(transform);
    CETECH_ADD_STATIC_MODULE(scenegraph);
    CETECH_ADD_STATIC_MODULE(scene);
    CETECH_ADD_STATIC_MODULE(mesh_renderer);
    CETECH_ADD_STATIC_MODULE(camera);

    CETECH_ADD_STATIC_MODULE(default_render_graph);
    CETECH_ADD_STATIC_MODULE(command_history);
    CETECH_ADD_STATIC_MODULE(property_inspector);

    CETECH_ADD_STATIC_MODULE(asset_property);
    CETECH_ADD_STATIC_MODULE(asset_preview);
    CETECH_ADD_STATIC_MODULE(texture_property);


    CETECH_ADD_STATIC_MODULE(log_view);
    CETECH_ADD_STATIC_MODULE(entity_editor);
    CETECH_ADD_STATIC_MODULE(entity_preview);
    CETECH_ADD_STATIC_MODULE(material_preview);
    CETECH_ADD_STATIC_MODULE(material_property);

}

