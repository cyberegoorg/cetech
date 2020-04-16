//==============================================================================
// Static engine system configuration
//==============================================================================

void init_static_modules() {
    CE_ADD_STATIC_MODULE(cdb_types);
    CE_ADD_STATIC_MODULE(metrics);
    CE_ADD_STATIC_MODULE(machine);

    CE_ADD_STATIC_MODULE(asset);
    CE_ADD_STATIC_MODULE(asset_io);

    CE_ADD_STATIC_MODULE(ecs);
    CE_ADD_STATIC_MODULE(parent);
    CE_ADD_STATIC_MODULE(node_graph);
    CE_ADD_STATIC_MODULE(node_test);
    CE_ADD_STATIC_MODULE(node_graph_editor);

    //==========================================================================
    // Renderer
    //==========================================================================
    CE_ADD_STATIC_MODULE(renderer);
    CE_ADD_STATIC_MODULE(shader);
    CE_ADD_STATIC_MODULE(texture);
    CE_ADD_STATIC_MODULE(material);
    CE_ADD_STATIC_MODULE(render_graph);
    CE_ADD_STATIC_MODULE(dd);

    //==========================================================================
    // Engine
    //==========================================================================

    CE_ADD_STATIC_MODULE(controlers);

    CE_ADD_STATIC_MODULE(keyboard);
    CE_ADD_STATIC_MODULE(gamepad);
    CE_ADD_STATIC_MODULE(mouse);

    CE_ADD_STATIC_MODULE(game_system);

    //==========================================================================
    // UI
    //==========================================================================
    CE_ADD_STATIC_MODULE(ui_imgui);

    //==========================================================================
    // Physics
    //==========================================================================
    CE_ADD_STATIC_MODULE(physics2d);
    CE_ADD_STATIC_MODULE(physics3d);
    CE_ADD_STATIC_MODULE(physics_box2d);
    CE_ADD_STATIC_MODULE(physics_bullet);

    //==========================================================================
    // Editor
    //==========================================================================
    CE_ADD_STATIC_MODULE(action_manager);
    CE_ADD_STATIC_MODULE(selected_object);
    CE_ADD_STATIC_MODULE(dock);

//    CE_ADD_STATIC_MODULE(profiler_dock);
    CE_ADD_STATIC_MODULE(playground);
    CE_ADD_STATIC_MODULE(asset_browser);
    CE_ADD_STATIC_MODULE(level_inspector);
    CE_ADD_STATIC_MODULE(property_inspector);

    CE_ADD_STATIC_MODULE(entity_property);
    CE_ADD_STATIC_MODULE(entity_explorer);

    CE_ADD_STATIC_MODULE(transform);
    CE_ADD_STATIC_MODULE(scene);
    CE_ADD_STATIC_MODULE(static_mesh);
    CE_ADD_STATIC_MODULE(primitive_mesh);
    CE_ADD_STATIC_MODULE(camera);

    CE_ADD_STATIC_MODULE(default_render_graph);

    CE_ADD_STATIC_MODULE(asset_preview);

    CE_ADD_STATIC_MODULE(log_view);
    CE_ADD_STATIC_MODULE(entity_editor);
    CE_ADD_STATIC_MODULE(editor);
    CE_ADD_STATIC_MODULE(game_view);
}

