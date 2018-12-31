//==============================================================================
// Static engine system configuration
//==============================================================================

void init_static_modules() {
    CE_ADD_STATIC_MODULE(machine);

    CE_ADD_STATIC_MODULE(builddb);
    CE_ADD_STATIC_MODULE(resourcesystem);
    CE_ADD_STATIC_MODULE(resourcecompiler);

    CE_ADD_STATIC_MODULE(ecs);

    //==========================================================================
    // Renderer
    //==========================================================================
    CE_ADD_STATIC_MODULE(renderer);
    CE_ADD_STATIC_MODULE(shader);
    CE_ADD_STATIC_MODULE(texture);
    CE_ADD_STATIC_MODULE(material);
    CE_ADD_STATIC_MODULE(debugdraw);
    CE_ADD_STATIC_MODULE(render_graph);

    //==========================================================================
    // Engine
    //==========================================================================

    CE_ADD_STATIC_MODULE(controlers);

    CE_ADD_STATIC_MODULE(keyboard);
    CE_ADD_STATIC_MODULE(gamepad);
    CE_ADD_STATIC_MODULE(mouse);

    CE_ADD_STATIC_MODULE(game_system);

    //==========================================================================
    // DebugUI
    //==========================================================================
    CE_ADD_STATIC_MODULE(debugui);

    //==========================================================================
    // Editor
    //==========================================================================
    CE_ADD_STATIC_MODULE(action_manager);
    CE_ADD_STATIC_MODULE(selected_object);
    CE_ADD_STATIC_MODULE(dock);

    CE_ADD_STATIC_MODULE(playground);
    CE_ADD_STATIC_MODULE(asset_browser);
    CE_ADD_STATIC_MODULE(level_inspector);
    CE_ADD_STATIC_MODULE(property_inspector);

    CE_ADD_STATIC_MODULE(entity_property);
    CE_ADD_STATIC_MODULE(entity_explorer);

    CE_ADD_STATIC_MODULE(transform);
    CE_ADD_STATIC_MODULE(scene);
    CE_ADD_STATIC_MODULE(mesh_renderer);
    CE_ADD_STATIC_MODULE(camera);

    CE_ADD_STATIC_MODULE(default_render_graph);

    CE_ADD_STATIC_MODULE(asset_preview);

    CE_ADD_STATIC_MODULE(log_view);
    CE_ADD_STATIC_MODULE(entity_editor);
    CE_ADD_STATIC_MODULE(editor);
    CE_ADD_STATIC_MODULE(sourcedb_ui);
    CE_ADD_STATIC_MODULE(game_view);
}

