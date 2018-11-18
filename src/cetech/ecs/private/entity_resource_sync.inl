static void _obj_to_prop(const struct ct_comp_prop_decs *comp_desc,
                         uint64_t obj,
                         uint64_t prop,
                         void *data) {
    const struct ct_prop_decs *decs = desc_by_name(comp_desc, prop);
    if (!decs) {
        return;
    }

    uint64_t offset = decs->offset;

    enum ct_ecs_prop_type prop_type = decs->type;
    switch (prop_type) {
        case ECS_PROP_FLOAT:
            *(float *) (data + offset) = ce_cdb_a0->read_float(obj,
                                                               prop,
                                                               0.0f);
            break;

        case ECS_PROP_VEC3: {
            ce_cdb_a0->read_vec3(obj, prop,
                                 ((float *) (data + offset)));
            break;
        }

        case ECS_PROP_RESOURCE_NAME:
            *(uint64_t *) (data + offset) = ce_cdb_a0->read_uint64(obj, prop,
                                                                   0);
            break;

        case ECS_PROP_STR_ID64 : {
            const char *str = ce_cdb_a0->read_str(obj, prop, 0);
            *(uint64_t *) (data + offset) = ce_id_a0->id64(str);
            break;
        }

        default:
            break;
    }
}

static void _obj_change_generic(struct ct_world world,
                                struct ct_component_i0 *ci,
                                uint64_t obj,
                                const uint64_t *props,
                                uint32_t prop_count,
                                struct ct_entity *ents,
                                uint32_t n) {
    const struct ct_comp_prop_decs *comp_desc = ci->prop_desc();

    for (int i = 0; i < n; ++i) {
        struct ct_entity ent = ents[i];
        void *data = ct_ecs_a0->component->get_one(world, ci->cdb_type(), ent);

        for (int p = 0; p < prop_count; ++p) {
            uint64_t prop = props[p];
            _obj_to_prop(comp_desc, obj, prop, data);
        }
    }
}

static void _on_component_obj_change(uint64_t obj,
                                     const uint64_t *prop,
                                     uint32_t prop_count,
                                     void *data) {
    struct ct_world world = {.h= (uint64_t) data};
    struct world_instance *w = get_world_instance(world);
    uint64_t idx = ce_hash_lookup(&w->component_objmap, obj, UINT64_MAX);

    if (UINT64_MAX == idx) {
        return;
    }

    struct spawn_info *info = &w->component_spawn_info[idx];


    uint64_t component_type = ce_cdb_a0->type(obj);

    struct ct_component_i0 *component_i;
    component_i = get_interface(component_type);

    if (!component_i) {
        return;
    }


    if (!component_i->obj_change) {

        _obj_change_generic(world, component_i, obj, prop, prop_count,
                            info->ents, ce_array_size(info->ents));

    } else {
        component_i->obj_change(world,
                                obj,
                                prop,
                                prop_count,
                                info->ents,
                                ce_array_size(info->ents));
    }
}


static void _on_components_obj_add(uint64_t obj,
                                   const uint64_t *prop,
                                   uint32_t prop_count,
                                   void *data) {
    struct ct_world world = {.h= (uint64_t) data};
    struct world_instance *w = get_world_instance(world);

    uint64_t idx = ce_hash_lookup(&w->obj_entmap, ce_cdb_a0->parent(obj),
                                  UINT64_MAX);

    if (UINT64_MAX == idx) {
        return;
    }

    struct spawn_info *info = &w->obj_spawn_info[idx];

    for (int i = 0; i < prop_count; ++i) {
        uint64_t component_type = prop[i];

        uint64_t comp_obj = ce_cdb_a0->read_subobject(obj, component_type, 0);
        ce_cdb_a0->register_notify(comp_obj, _on_component_obj_change,
                                   (void *) world.h);

        const uint32_t ent_n = ce_array_size(info->ents);
        for (int j = 0; j < ent_n; ++j) {
            add_components(world, info->ents[j], &component_type, 1, NULL);
            _add_spawn_component_obj(w, comp_obj, info->ents[j]);
        }
    }
}

static void _on_components_obj_removed(uint64_t obj,
                                       const uint64_t *prop,
                                       uint32_t prop_count,
                                       void *data) {

    for (int i = 0; i < prop_count; ++i) {
        uint64_t component_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);

        struct ct_world world = {.h= (uint64_t) data};
        struct world_instance *w = get_world_instance(world);
        uint64_t idx = ce_hash_lookup(&w->component_objmap, component_obj,
                                      UINT64_MAX);

        if (UINT64_MAX == idx) {
            return;
        }

        struct spawn_info *info = &w->component_spawn_info[idx];

        const uint32_t ent_n = ce_array_size(info->ents);
        for (int j = 0; j < ent_n; ++j) {
            remove_components(world, info->ents[j], &prop[i], 1);
        }
    }
}

static void _on_entity_obj_removed(uint64_t obj,
                                   const uint64_t *prop,
                                   uint32_t prop_count,
                                   void *data) {

    for (int i = 0; i < prop_count; ++i) {
        uint64_t entity_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);

        struct ct_world world = {.h= (uint64_t) data};
        struct world_instance *w = get_world_instance(world);

        uint64_t idx = ce_hash_lookup(&w->obj_entmap, entity_obj, UINT64_MAX);

        if (idx == UINT64_MAX) {
            continue;
        }

        struct spawn_info *info = &w->obj_spawn_info[idx];

        const uint32_t ent_n = ce_array_size(info->ents);
        destroy(world, info->ents, ent_n);
    }
}


static void _on_entity_obj_add(uint64_t obj,
                               const uint64_t *prop,
                               uint32_t prop_count,
                               void *data) {

    for (int i = 0; i < prop_count; ++i) {
        uint64_t entity_obj = ce_cdb_a0->read_subobject(obj, prop[i], 0);

        struct ct_world world = {.h= (uint64_t) data};
        struct world_instance *w = get_world_instance(world);

        struct ct_entity root_ent;
        create_entities(world, &root_ent, 1);

        _add_spawn_entity_obj(w, entity_obj, root_ent);

        uint64_t components;
        components = ce_cdb_a0->read_subobject(entity_obj, ENTITY_COMPONENTS,
                                               0);

        if (components) {
            ce_cdb_a0->register_remove_notify(components,
                                              _on_components_obj_removed,
                                              (void *) world.h);

            ce_cdb_a0->register_notify(components, _on_components_obj_add,
                                       (void *) world.h);
        }

        uint64_t children;
        children = ce_cdb_a0->read_subobject(entity_obj, ENTITY_CHILDREN, 0);

        if (children) {
            ce_cdb_a0->register_remove_notify(children, _on_entity_obj_removed,
                                              (void *) world.h);

            ce_cdb_a0->register_notify(children, _on_entity_obj_add,
                                       (void *) world.h);
        }

    }
}

///

static void _collect_keys(struct ct_resource_id rid,
                          uint64_t obj,
                          ce_cdb_obj_o *w,
                          uint32_t idx) {
    if (ce_cdb_a0->read_uint64(obj, ASSET_NAME, 0) == rid.name) {
        return;
    }

    uint64_t k = ce_cdb_a0->key(obj);
    ce_cdb_a0->set_uint64(w, idx, k);

    uint64_t parent = ce_cdb_a0->parent(obj);
    _collect_keys(rid, parent, w, idx + 1);
}

static uint64_t _find_recursive_create(uint64_t obj,
                                       uint64_t keys) {
    uint64_t n = ce_cdb_a0->prop_count(keys);

    uint64_t it_obj = obj;
    for (int i = 0; i < n; ++i) {
        uint64_t k = ce_cdb_a0->read_uint64(keys, n - 1 - i, 0);

        if (!ce_cdb_a0->prop_exist_norecursive(it_obj, k)) {
            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(it_obj);
            it_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
            ce_cdb_a0->set_subobject(w, k, it_obj);
            ce_cdb_a0->write_commit(w);
        } else {
            it_obj = ce_cdb_a0->read_subobject(it_obj, k, 0);
        }
    }

    return it_obj;
}

static void entity_resource_changed(uint64_t asset_obj,
                                    uint64_t obj,
                                    const uint64_t *prop,
                                    uint32_t prop_count) {
    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = asset_name,
            .type = ENTITY_RESOURCE_ID,
    };

    uint64_t keys = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(keys);
    _collect_keys(rid, obj, w, 0);
    ce_cdb_a0->write_commit(w);

    uint64_t resource_obj = ct_resource_a0->get(rid);
    uint64_t robj = _find_recursive_create(resource_obj, keys);
    w = ce_cdb_a0->write_begin(robj);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t p = prop[i];
        enum ce_cdb_type type = ce_cdb_a0->prop_type(obj, p);

        switch (type) {
            case CDB_TYPE_UINT64: {
                uint64_t ui = ce_cdb_a0->read_uint64(obj, p, 0);
                ce_cdb_a0->set_uint64(w, p, ui);
            }
                break;


            case CDB_TYPE_REF: {
                uint64_t ui = ce_cdb_a0->read_uint64(obj, p, 0);
                ce_cdb_a0->set_ref(w, p, ui);
            }
                break;

            case CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(obj, p, 0.0f);
                ce_cdb_a0->set_float(w, p, f);
            }
                break;

            case CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(obj, p, false);
                ce_cdb_a0->set_bool(w, p, b);
            }
                break;

            case CDB_TYPE_STR: {
                const char *str = ce_cdb_a0->read_str(obj, p, "");
                ce_cdb_a0->set_str(w, p, str);
            }

                break;
            case CDB_TYPE_VEC3: {
                float v[3] = {};
                ce_cdb_a0->read_vec3(obj, p, v);
                ce_cdb_a0->set_vec3(w, p, v);
            }
                break;

            case CDB_TYPE_VEC4: {
                float v[4] = {};
                ce_cdb_a0->read_vec4(obj, p, v);
                ce_cdb_a0->set_vec4(w, p, v);
            }

                break;

            case CDB_TYPE_SUBOBJECT: {
                uint64_t subobj;
                subobj = ce_cdb_a0->read_subobject(obj, p, 0);

                uint64_t new_subobj = ce_cdb_a0->create_from(ce_cdb_a0->db(),
                                                             subobj);

                ce_cdb_a0->set_subobject(w, p, new_subobj);
            }
                break;

            case CDB_TYPE_MAT4:
                break;
            case CDB_TYPE_NONE:
                break;
            case CDB_TYPE_PTR:
                break;
            case CDB_TYPE_BLOB:
                break;
        }

    }

    ce_cdb_a0->write_commit(w);
}

static void entity_resource_removed(uint64_t asset_obj,
                                    uint64_t obj,
                                    const uint64_t *prop,
                                    uint32_t prop_count) {
    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, ASSET_NAME, 0);

    struct ct_resource_id rid = {
            .name = asset_name,
            .type = ENTITY_RESOURCE_ID,
    };

    uint64_t keys = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(keys);
    _collect_keys(rid, obj, w, 0);
    ce_cdb_a0->write_commit(w);

    uint64_t resource_obj = ct_resource_a0->get(rid);
    uint64_t robj = _find_recursive_create(resource_obj, keys);
    w = ce_cdb_a0->write_begin(robj);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t p = prop[i];
        ce_cdb_a0->remove_property(w, p);
    }

    ce_cdb_a0->write_commit(w);
}
