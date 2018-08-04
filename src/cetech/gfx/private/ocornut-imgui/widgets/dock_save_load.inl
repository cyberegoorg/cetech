
        void saveToYaml(char** buffer, ct_alloc* alloc) {
            static const char *DOCK_ENTRY_TEMPLATE = "%d: \n"
                    "  index: %d\n"
                    "  label: %s\n"
                    "  x: %d\n"
                    "  y: %d\n"
                    "  location: %s\n"
                    "  size_x: %d\n"
                    "  size_y: %d\n"
                    "  status: %d\n"
                    "  active: %d\n"
                    "  opened: %d\n"
                    "  prev: %d\n"
                    "  next: %d\n"
                    "  child0: %d\n"
                    "  child1: %d\n"
                    "  parent: %d\n";

            for (int i = 0; i < m_docks.size(); ++i) {
                Dock &dock = *m_docks[i];

                const char* label = strlen(dock.label) > 0 ? dock.label : "\"\"";
                const char* location = strlen(dock.location) > 0 ? dock.location : "\"\"";

                ct_buffer_printf(buffer,alloc,
                                 DOCK_ENTRY_TEMPLATE,
                                 i,
                                 i,
                                 label,
                                 (int) dock.pos.x,
                                 (int) dock.pos.y,
                                 location,
                                 (int) dock.size.x,
                                 (int) dock.size.y,
                                 (int) dock.status,
                                 (int) dock.active,
                                 (int) dock.opened,
                                 (int) getDockIndex(dock.prev_tab),
                                 (int) getDockIndex(dock.next_tab),
                                 (int) getDockIndex(dock.children[0]),
                                 (int) getDockIndex(dock.children[1]),
                                 (int) getDockIndex(dock.parent));
            }
        }

        void loadFromYaml(const char* path, ct_ydb_a0* ydb, ct_yng_a0* yng) {
            const uint32_t size = m_docks.size();
            for (int i = 0; i < size; ++i) {
                m_docks[i]->~Dock();
                MemFree(m_docks[i]);
            }
            m_docks.clear();

            uint64_t tmp_keys = 0;
            uint64_t type_keys[32] = {};
            uint32_t type_keys_count = 0;
            ydb->get_map_keys(path,
                              &tmp_keys,1,
                              type_keys,CE_ARRAY_LEN(type_keys),
                              &type_keys_count);

            for (uint32_t i = 0; i < type_keys_count; ++i) {
                Dock *new_dock = (Dock *) MemAlloc(sizeof(Dock));
                m_docks.push_back(IM_PLACEMENT_NEW(new_dock) Dock());
            }

            for (uint32_t i = 0; i < type_keys_count; ++i) {
                uint64_t k[2] {
                        type_keys[i],
                        yng->key("index")
                };

                int index_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("label");
                const char* label_n = ydb->get_str(path, k, 2, "");

                k[1] = yng->key("x");
                int x_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("y");
                int y_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("location");
                const char* location_n = ydb->get_str(path, k, 2, "");

                k[1] = yng->key("size_x");
                int size_x_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("size_y");
                int size_y_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("status");
                int status_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("active");
                int active_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("opened");
                int opened_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("prev");
                int prev_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("next");
                int next_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("child0");
                int child0_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("child1");
                int child1_n = (int) ydb->get_float(path, k, 2, 0);

                k[1] = yng->key("parent");
                int parent_n = (int) ydb->get_float(path, k, 2, 0);

                Dock* dock =  this->getDockByIndex(index_n);

                dock->label = ImStrdup(label_n);
                dock->id = ImHash(dock->label, 0);
                strcpy(dock->location, location_n);

                dock->pos.x = x_n;
                dock->pos.y =   y_n;
                dock->size.x =  size_x_n;
                dock->size.y =  size_y_n;
                dock->status =  Status_ (status_n);
                dock->active =  active_n != 0;
                dock->opened =  opened_n != 0;

                dock->prev_tab = this->getDockByIndex(prev_n);
                dock->next_tab = this->getDockByIndex(next_n);
                dock->children[0] = this->getDockByIndex(child0_n);
                dock->children[1] = this->getDockByIndex(child1_n);
                dock->parent = this->getDockByIndex(parent_n);

                tryDockToStoredLocation(*dock);
            }
        }
    };

    IMGUI_API void saveToYaml(char** buffer, ct_alloc* alloc) {
        s_dock->saveToYaml(buffer, alloc);
    }

    IMGUI_API void loadFromYaml(const char* path, ct_ydb_a0* ydb, ct_yng_a0* yng) {
        s_dock->loadFromYaml(path, ydb, yng);
    }


} // namespace ImGui
