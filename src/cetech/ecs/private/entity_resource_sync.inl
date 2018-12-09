//void _on_component_obj_change(uint64_t obj,
//                              const uint64_t *prop,
//                              uint32_t prop_count,
//                              void *data) {
//    struct ct_world world = {.h= (uint64_t) data};
//    struct world_instance *w = get_world_instance(world);
//    uint64_t idx = ce_hash_lookup(&w->component_objmap, obj, UINT64_MAX);
//
//    if (UINT64_MAX == idx) {
//        return;
//    }
//
//    struct spawn_info *info = &w->component_spawn_info[idx];
//
//
//    uint64_t component_type = ce_cdb_a0->obj_type(obj);
//
//    struct ct_component_i0 *component_i;
//    component_i = get_interface(component_type);
//
//    if (!component_i) {
//        return;
//    }
//
//
//    if (component_i->obj_change) {
//        component_i->obj_change(world,
//                                obj,
//                                prop,
//                                prop_count,
//                                info->ents,
//                                ce_array_size(info->ents));
//    }
//}
//
//void _on_components_obj_add(uint64_t obj,
//                            const uint64_t *prop,
//                            uint32_t prop_count,
//                            void *data) {
//    struct ct_world world = {.h= (uint64_t) data};
//    struct world_instance *w = get_world_instance(world);
//
//    uint64_t idx = ce_hash_lookup(&w->obj_entmap, ce_cdb_a0->parent(obj),
//                                  UINT64_MAX);
//
//    if (UINT64_MAX == idx) {
//        return;
//    }
//
//    struct spawn_info *info = &w->obj_spawn_info[idx];
//
//    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);
//    for (int i = 0; i < prop_count; ++i) {
//        uint64_t component_type = prop[i];
//
//        uint64_t comp_obj = ce_cdb_a0->read_subobject(reader, component_type, 0);
//
//        struct ct_component_i0 *component_i;
//        component_i = get_interface(component_type);
//
//        if (!component_i) {
//            continue;
//        }
//
//        const uint32_t ent_n = ce_array_size(info->ents);
//        for (int j = 0; j < ent_n; ++j) {
//            add_components(world, info->ents[j], &component_type, 1, NULL);
//            _add_spawn_component_obj(w, comp_obj, info->ents[j]);
//
////            void *data = get_one(world, component_type, info->ents[j]);
//            if (component_i->spawner) {
//                component_i->spawner(world, comp_obj);
//            }
//        }
//    }
//}
//
//void _on_components_obj_removed(uint64_t obj,
//                                const uint64_t *prop,
//                                uint32_t prop_count,
//                                void *data) {
//    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);
//
//    for (int i = 0; i < prop_count; ++i) {
//        uint64_t component_obj = ce_cdb_a0->read_subobject(reader, prop[i], 0);
//
//        struct ct_world world = {.h= (uint64_t) data};
//        struct world_instance *w = get_world_instance(world);
//        uint64_t idx = ce_hash_lookup(&w->component_objmap, component_obj,
//                                      UINT64_MAX);
//
//        if (UINT64_MAX == idx) {
//            return;
//        }
//
//        struct spawn_info *info = &w->component_spawn_info[idx];
//
//        const uint32_t ent_n = ce_array_size(info->ents);
//        for (int j = 0; j < ent_n; ++j) {
//            remove_components(world, info->ents[j], &prop[i], 1);
//        }
//    }
//}
//
//void _on_entity_obj_removed(uint64_t obj,
//                            const uint64_t *prop,
//                            uint32_t prop_count,
//                            void *data) {
//    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);
//
//    for (int i = 0; i < prop_count; ++i) {
//        uint64_t entity_obj = ce_cdb_a0->read_subobject(reader, prop[i], 0);
//
//        struct ct_world world = {.h= (uint64_t) data};
//        struct world_instance *w = get_world_instance(world);
//
//        uint64_t idx = ce_hash_lookup(&w->obj_entmap, entity_obj, UINT64_MAX);
//
//        if (idx == UINT64_MAX) {
//            continue;
//        }
//
//        struct spawn_info *info = &w->obj_spawn_info[idx];
//
//        const uint32_t ent_n = ce_array_size(info->ents);
//        destroy(world, info->ents, ent_n);
//    }
//}
//
//
//void _on_entity_obj_add(uint64_t obj,
//                        const uint64_t *prop,
//                        uint32_t prop_count,
//                        void *data) {
//    const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);
//
//    for (int i = 0; i < prop_count; ++i) {
//        uint64_t entity_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);
//
//        struct ct_world world = {.h= (uint64_t) data};
//        struct world_instance *w = get_world_instance(world);
//
//        struct ct_entity root_ent;
//        create_entities(world, &root_ent, 1);
//
//        _add_spawn_entity_obj(w, entity_obj, root_ent);
//    }
//}
//

static void on_post_update(uint64_t type,
                           void *event) {
//    struct ebus_event_header *it = ce_ebus_a0->events(CDB_EBUS);
//    struct ebus_event_header *end_it = ce_ebus_a0->events_end(CDB_EBUS);
//
//    struct world_instance *world_array = _G.world_array;
//    uint64_t wordl_n = ce_array_size(world_array);
//
//    while (it != end_it) {
//        if (it->type == CDB_PROP_CHANGED_EVENT) {
//            struct ce_cdb_prop_ev0 *ev = CE_EBUS_BODY(it);
//            uint64_t k = ce_cdb_a0->obj_key(ev->obj);
//
//            if (k == ENTITY_CHILDREN) {
//                for (int i = 0; i < wordl_n; ++i) {
//                    struct world_instance *world = &world_array[i];
//                    _on_entity_obj_add(ev->obj, ev->prop,
//                                       ev->prop_count,
//                                       (void *) world->world.h);
//                }
//            } else if (k == ENTITY_COMPONENTS) {
//                for (int i = 0; i < wordl_n; ++i) {
//                    struct world_instance *world = &world_array[i];
//                    _on_components_obj_add(ev->obj, ev->prop,
//                                           ev->prop_count,
//                                           (void *) world->world.h);
//                }
//            } else {
//                for (int i = 0; i < wordl_n; ++i) {
//                    struct world_instance *world = &world_array[i];
//                    _on_component_obj_change(ev->obj, ev->prop,
//                                             ev->prop_count,
//                                             (void *) world->world.h);
//                }
//            }
//
//        } else if (it->type == CDB_PROP_REMOVED_EVENT) {
//            struct ce_cdb_prop_ev0 *ev = CE_EBUS_BODY(it);
//            uint64_t k = ce_cdb_a0->obj_key(ev->obj);
//
//            if (k == ENTITY_CHILDREN) {
//                for (int i = 0; i < wordl_n; ++i) {
//                    struct world_instance *world = &world_array[i];
//                    _on_entity_obj_removed(ev->obj, ev->prop,
//                                           ev->prop_count,
//                                           (void *) world->world.h);
//                }
//            } else if (k== ENTITY_COMPONENTS) {
//                for (int i = 0; i < wordl_n; ++i) {
//                    struct world_instance *world = &world_array[i];
//                    _on_components_obj_removed(ev->obj, ev->prop,
//                                               ev->prop_count,
//                                               (void *) world->world.h);
//                }
//            }
//        }
//
//        it = CE_EBUS_NEXT(it);
//    }
}