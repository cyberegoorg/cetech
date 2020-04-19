#include <celib/api.h>
#include <celib/config.h>
#include <celib/memory/memory.h>
#include <celib/module.h>
#include <celib/cdb_yaml.h>
#include <celib/id.h>
#include <celib/log.h>
#include <celib/cdb_yaml.h>
#include <celib/fs.h>
#include <celib/memory/allocator.h>
#include <celib/math/math.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/cdb.h>
#include <celib/os/thread.h>
#include <celib/os/vio.h>
#include <celib/uuid64.h>

#include <yaml/yaml.h>

#define _G ydb_global
#define LOG_WHERE "ydb"

enum node_type {
    NODE_INVALID = 0,
    NODE_FLOAT,
    NODE_UINT,
    NODE_REF,
    NODE_STRING,
    NODE_TRUE,
    NODE_FALSE,
    NODE_MAP,
    NODE_SEQ,
};

typedef struct node_value {
    union {
        float f;
        uint64_t ui;
        char *string;
        ce_cdb_uuid_t0 uuid;
    };
} node_value;


static struct _G {
    ce_alloc_t0 *allocator;
} _G;


static bool _is_ref(const char *str) {
    ce_uuid64_t0 uuid;
    if (!ce_uuid64_try_parse(&uuid, str)) {
        return (str[0] == '0') && (str[1] == 'x');
    }

    return true;
}

static ce_cdb_uuid_t0 _uid_from_str(const char *str) {
    ce_uuid64_t0 uuid;
    if (!ce_uuid64_try_parse(&uuid, str)) {
        uint64_t v = strtoul(str, NULL, 0);
        return (ce_cdb_uuid_t0) {.id=v};
    }

    return *((ce_cdb_uuid_t0 *) (&uuid));
}

static void type_value_from_scalar(const uint8_t *scalar,
                                   enum node_type *type,
                                   struct node_value *vallue,
                                   bool is_key) {
    const char *scalar_str = (const char *) scalar;

    if (!strlen(scalar_str)) {
        *type = NODE_STRING;
        *vallue = (node_value) {.string = strdup(scalar_str)};
        return;
    }

    float f;
    uint64_t ui;

    if (!is_key) {
        if (!((scalar_str[0] == '0') && (scalar_str[1] == 'x'))) {
            if ((strchr(scalar_str, '.') != NULL) && sscanf(scalar_str, "%f", &f)) {
                *type = NODE_FLOAT;
                *vallue = (node_value) {.f = f};
                return;
            } else if (sscanf(scalar_str, "%llu", &ui)) {
                *type = NODE_UINT;
                *vallue = (node_value) {.ui = ui};
                return;

            } else if ((0 == strcmp(scalar_str, "y")) ||
                       (0 == strcmp(scalar_str, "yes")) ||
                       (0 == strcmp(scalar_str, "true"))) {
                *type = NODE_TRUE;
                return;
            } else if ((0 == strcmp(scalar_str, "n")) ||
                       (0 == strcmp(scalar_str, "no")) ||
                       (0 == strcmp(scalar_str, "false"))) {
                *type = NODE_FALSE;
                return;
            }
        }

    } else if (_is_ref(scalar_str)) {
        ce_cdb_uuid_t0 uuid = _uid_from_str(scalar_str);
        *type = NODE_REF;
        *vallue = (node_value) {.uuid = uuid};
        return;
    }

    *type = NODE_STRING;
    *vallue = (node_value) {.string = strdup(scalar_str)};
}

typedef struct parent_stack_state {
    enum node_type type;
    uint32_t node_count;

    uint64_t key_hash;
    uint64_t str_hash;

    uint32_t CT_CDB_NODE_idx;
} parent_stack_state;

static uint32_t _push_cnode(ct_cdb_node_t **nodes,
                            ct_cdb_node_t node) {
    uint32_t idx = ce_array_size(*nodes);
    ce_array_push(*nodes, node, ce_memory_a0->system);
    return idx;
}

ce_cdb_uuid_t0 cnodes_from_vio(ce_vio_t0 *vio,
                               ct_cdb_node_t **cnodes,
                               struct ce_alloc_t0 *alloc) {

    parent_stack_state *parent_stack = NULL;
    uint32_t parent_stack_top;
    uint64_t key = 0;

    ce_array_push(parent_stack, ((parent_stack_state) {}), _G.allocator);

    uint8_t *source_data = CE_ALLOC(alloc, uint8_t, vio->vt->size(vio->inst) + 1);
    memset(source_data, 0, vio->vt->size(vio->inst) + 1);
    vio->vt->read(vio->inst, source_data, sizeof(char), vio->vt->size(vio->inst));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        ce_log_a0->error(LOG_WHERE, "Failed to initialize parser");
        yaml_parser_delete(&parser);

        return (ce_cdb_uuid_t0) {};
    }

    yaml_parser_set_input_string(&parser, source_data, vio->vt->size(vio->inst));

#define IS_KEY() (parent_stack[parent_stack_top].type == NODE_MAP)
#define HAS_KEY() (parent_stack[parent_stack_top].type == NODE_STRING) || (parent_stack[parent_stack_top].type == NODE_REF)
    yaml_event_t event;

    do {
        parent_stack_top = ce_array_size(parent_stack) - 1;

        if (!yaml_parser_parse(&parser, &event)) {
            ce_log_a0->error(LOG_WHERE, "Parser error %d\n", parser.error);
            yaml_event_delete(&event);
            yaml_parser_delete(&parser);

            return (ce_cdb_uuid_t0) {};
        }

        struct parent_stack_state state = {};
        switch (event.type) {
            case YAML_NO_EVENT:
                break;

            case YAML_STREAM_START_EVENT:
                break;

            case YAML_STREAM_END_EVENT:
                break;

            case YAML_DOCUMENT_START_EVENT:
                break;

            case YAML_DOCUMENT_END_EVENT:
                break;

            case YAML_SEQUENCE_START_EVENT: {
                key = parent_stack[parent_stack_top].str_hash;
                state = (parent_stack_state) {
                        .type = NODE_SEQ,
                        .key_hash = key,
                };

                ++parent_stack[parent_stack_top].node_count;

                if (HAS_KEY()) {
                    ce_array_pop_back(parent_stack);
                }

                ce_array_push(parent_stack, state, _G.allocator);
                break;
            }

            case YAML_MAPPING_START_EVENT: {
                key = parent_stack[parent_stack_top].str_hash;

                uint32_t CT_CDB_NODE_idx = _push_cnode(cnodes, (ct_cdb_node_t) {
                        .type = CT_CDB_NODE_OBJ_BEGIN,
                });

                state = (parent_stack_state) {
                        .type = NODE_MAP,
                        .key_hash = key,
                        .CT_CDB_NODE_idx = CT_CDB_NODE_idx,
                };

                ++parent_stack[parent_stack_top].node_count;

                if (HAS_KEY()) {
                    state.key_hash = parent_stack[parent_stack_top].str_hash;
                    ce_array_pop_back(parent_stack);

                    (*cnodes)[CT_CDB_NODE_idx].key = parent_stack[parent_stack_top].str_hash;
                }

                ce_array_push(parent_stack, state, _G.allocator);
                break;
            }


            case YAML_MAPPING_END_EVENT: {
                ce_array_pop_back(parent_stack);

                uint32_t CT_CDB_NODE_idx = parent_stack[parent_stack_top].CT_CDB_NODE_idx;
                ct_cdb_node_t *cnode = &(*cnodes)[CT_CDB_NODE_idx];

                if (cnode->type == CT_CDB_NODE_OBJSET) {
                    _push_cnode(cnodes, (ct_cdb_node_t) {
                            .type = CT_CDB_NODE_OBJSET_END
                    });
                } else {
                    if (!cnode->obj.uuid.id) {
                        cnode->obj.uuid = ce_cdb_a0->gen_uid(ce_cdb_a0->db());
                    }
                    _push_cnode(cnodes, (ct_cdb_node_t) {
                            .type = CT_CDB_NODE_OBJ_END,
                    });
                }

                break;
            }

            case YAML_ALIAS_EVENT:
                break;

            case YAML_SCALAR_EVENT: {
                enum node_type type;
                struct node_value value;
                type_value_from_scalar(event.data.scalar.value, &type, &value, IS_KEY());

                if (IS_KEY()) {

                    uint64_t key_hash = 0;

                    if (type == NODE_REF) {
                        key_hash = value.ui;
                    } else {
                        key_hash = ce_id_a0->id64(value.string);
                    }

                    state = (parent_stack_state) {
                            .type = NODE_STRING,
                            .str_hash = key_hash,
                            .key_hash=key_hash
                    };

                    ++parent_stack[parent_stack_top].node_count;
                    ce_array_push(parent_stack, state, _G.allocator);


                    // VALUE_WITH_KEY
                } else if (parent_stack[parent_stack_top].type == NODE_STRING) {
                    key = parent_stack[parent_stack_top].str_hash;

                    switch (type) {
                        case NODE_FLOAT:
                            _push_cnode(cnodes, (ct_cdb_node_t) {
                                    .type = CT_CDB_NODE_FLOAT,
                                    .key = key,
                                    .value.f = value.f,
                            });

                            break;

                        case NODE_UINT:
                            _push_cnode(cnodes, (ct_cdb_node_t) {
                                    .type = CT_CDB_NODE_UINT,
                                    .key = key,
                                    .value.uint64 = value.ui
                            });

                            break;

                        case NODE_STRING:
                            if (key == CDB_UID_PROP) {
                                uint32_t CT_CDB_NODE_idx = parent_stack[parent_stack_top -
                                                                        1].CT_CDB_NODE_idx;
                                ct_cdb_node_t *cnode = &(*cnodes)[CT_CDB_NODE_idx];

                                ce_cdb_uuid_t0 uuid = _uid_from_str(value.string);
                                cnode->obj.uuid = uuid;
                            } else if (key == CDB_TYPE_PROP) {
                                uint32_t CT_CDB_NODE_idx = parent_stack[parent_stack_top -
                                                                        1].CT_CDB_NODE_idx;
                                ct_cdb_node_t *cnode = &(*cnodes)[CT_CDB_NODE_idx];

                                uint64_t type = 0;

                                if (_is_ref(value.string)) {
                                    type = _uid_from_str(value.string).id;
                                } else {
                                    type = ce_id_a0->id64(value.string);
                                }

                                if (type == CDB_OBJSET) {
                                    cnode->type = CT_CDB_NODE_OBJSET;
                                } else {
                                    cnode->obj.type = type;
                                }

                            } else if (key == CDB_INSTANCE_PROP) {
                                uint32_t CT_CDB_NODE_idx = parent_stack[parent_stack_top -
                                                                        1].CT_CDB_NODE_idx;
                                ct_cdb_node_t *cnode = &(*cnodes)[CT_CDB_NODE_idx];

                                ce_cdb_uuid_t0 uuid = _uid_from_str(value.string);
                                cnode->obj.instance_of = uuid;
                            } else if (_is_ref(value.string)) {
                                ce_cdb_uuid_t0 uuid = _uid_from_str(value.string);

                                _push_cnode(cnodes, (ct_cdb_node_t) {
                                        .type = CT_CDB_NODE_REF,
                                        .key = key,
                                        .uuid = uuid,
                                });

                            } else {
                                _push_cnode(cnodes, (ct_cdb_node_t) {
                                        .type = CT_CDB_NODE_STRING,
                                        .key = key,
                                        .value.str = value.string
                                });
                            }

                            break;

                        case NODE_TRUE:
                            _push_cnode(cnodes, (ct_cdb_node_t) {
                                    .type = CT_CDB_NODE_BOOL,
                                    .key = key,
                                    .value.b = true
                            });

                            break;

                        case NODE_FALSE:
                            _push_cnode(cnodes, (ct_cdb_node_t) {
                                    .type = CT_CDB_NODE_BOOL,
                                    .key = key,
                                    .value.b = false
                            });

                            break;

                        case NODE_REF:
                        case NODE_INVALID:
                        case NODE_MAP:
                        case NODE_SEQ:
                            break;
                    }

                    ce_array_pop_back(parent_stack);
                }

            }
                break;
            default:
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        }

    } while (event.type != YAML_STREAM_END_EVENT);

    yaml_event_delete(&event);
    yaml_parser_delete(&parser);

    return (*cnodes)[0].obj.uuid;
}

void _log_cnodes(ct_cdb_node_t *cnodes) {
    uint32_t cnodes_n = ce_array_size(cnodes);
    for (int j = 0; j < cnodes_n; ++j) {
        ct_cdb_node_t node = cnodes[j];

        switch (node.type) {
            default:
            case CT_CDB_NODE_INVALID: {
            }
                break;
            case CT_CDB_NODE_FLOAT: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %f",
                                 ce_id_a0->str_from_id64(node.key), node.value.f);
            }
                break;
            case CT_CDB_NODE_UINT: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %llu",
                                 ce_id_a0->str_from_id64(node.key), node.value.uint64);
            }
                break;
            case CT_CDB_NODE_REF: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %llx",
                                 ce_id_a0->str_from_id64(node.key), node.value.ref);
            }
                break;
            case CT_CDB_NODE_STRING: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %s",
                                 ce_id_a0->str_from_id64(node.key), node.value.str);
            }
                break;
            case CT_CDB_NODE_BOOL: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %s",
                                 ce_id_a0->str_from_id64(node.key),
                                 node.value.b ? "true" : "false");
            }
                break;
            case CT_CDB_NODE_OBJ_BEGIN: {
                ce_log_a0->debug(LOG_WHERE, "OBJ_BEGIN");

                if (node.key) {
                    ce_log_a0->debug(LOG_WHERE, "key: %s", ce_id_a0->str_from_id64(node.key));
                }

                ce_log_a0->debug(LOG_WHERE, "id: %llx", node.obj.uuid.id);
                if (node.obj.instance_of.id) {
                    ce_log_a0->debug(LOG_WHERE, "inst: %llx", node.obj.instance_of.id);
                }
            }
                break;
            case CT_CDB_NODE_OBJ_END: {
                ce_log_a0->debug(LOG_WHERE, "OBJ_END");
            }
                break;
            case CT_CDB_NODE_OBJSET: {
                ce_log_a0->debug(LOG_WHERE, "\tOBJSET %s",
                                 ce_id_a0->str_from_id64(node.key));
            }
                break;
            case CT_CDB_NODE_OBJSET_END: {
                ce_log_a0->debug(LOG_WHERE, "\tOBJSET_END");
            }
                break;
        }
    }
}

uint64_t _load_obj(const char *path,
                   ct_cdb_node_t **cnodes,
                   uint64_t path_key) {

    const uint64_t fs_root = ce_id_a0->id64("source");

    ce_log_a0->debug(LOG_WHERE, "Load file %s", path);

    ce_vio_t0 *f = ce_fs_a0->open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return 0;
    }

    ce_cdb_uuid_t0 uuid = cnodes_from_vio(f, cnodes, _G.allocator);
    ce_fs_a0->close(f);

    if (!uuid.id) {
        ce_log_a0->error(LOG_WHERE, "Could not load file %s", path);
        return 0;
    }

    return uuid.id;
}

uint64_t load_obj(const char *path) {
    uint64_t path_key = ce_id_a0->id64(path);

    ct_cdb_node_t *cnodes = NULL;
    _load_obj(path, &cnodes, path_key);
    uint64_t obj = ce_cdb_a0->load_from_cnodes(cnodes, ce_cdb_a0->db());
    ce_array_free(cnodes, _G.allocator);
    return obj;
}


static void _push_space(char **buffer,
                        uint32_t level) {
    for (int j = 0; j < level; ++j) {
        ce_buffer_printf(buffer, _G.allocator, "  ");
    }
}

static void _save_to_buffer(ce_cdb_t0 _db,
                            char **buffer,
                            uint64_t from,
                            uint32_t level) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(_db, from);

    const uint32_t prop_count = ce_cdb_a0->prop_count(r);
    const uint64_t *keys = ce_cdb_a0->prop_keys(r);

    _push_space(buffer, level);

    const ce_cdb_uuid_t0 uuid = ce_cdb_a0->obj_uid(_db, from);
    char uuid_buffer[64] = {};
    ce_uuid64_to_string(uuid_buffer, CE_ARRAY_LEN(uuid_buffer), (const ce_uuid64_t0 *) &uuid);

    ce_buffer_printf(buffer, _G.allocator, "cdb_uuid: %s\n", uuid_buffer);

    uint64_t type = ce_cdb_a0->obj_type(_db, from);
    const char *type_str = ce_id_a0->str_from_id64(type);
    if (type_str) {
        _push_space(buffer, level);
        ce_buffer_printf(buffer, _G.allocator, "cdb_type: %s\n", type_str);
    } else {
        if (type) {
            _push_space(buffer, level);
            ce_buffer_printf(buffer, _G.allocator, "cdb_type: 0x%llx\n", type);
        }
    }

    uint64_t instance_of = ce_cdb_a0->read_instance_of(r);
    if (instance_of) {
        const ce_cdb_uuid_t0 instance_of_uuid = ce_cdb_a0->obj_uid(_db, instance_of);

        _push_space(buffer, level);

        ce_uuid64_to_string(uuid_buffer, CE_ARRAY_LEN(uuid_buffer),
                            (const ce_uuid64_t0 *) &instance_of_uuid);

        ce_buffer_printf(buffer, _G.allocator, "cdb_instance: %s\n", uuid_buffer);
    }

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];

        if (key == CDB_INSTANCE_PROP) {
            continue;
        }

        if (instance_of) {
            const ce_cdb_obj_o0 *ir = ce_cdb_a0->read(_db, instance_of);
            if (ce_cdb_a0->prop_equal(ir, r, key)) {
                continue;
            }
        }

        const char *k = ce_id_a0->str_from_id64(key);

        ce_cdb_type_e0 type = ce_cdb_a0->prop_type(r, key);


        if ((type == CE_CDB_TYPE_BLOB) || (type == CE_CDB_TYPE_PTR)) {
//        if (type == CE_CDB_TYPE_PTR) {
            continue;
        }


        _push_space(buffer, level);

        const char *sufix = "";

        if (k) {
            ce_buffer_printf(buffer, _G.allocator, "%s%s:", k, sufix);
        } else {
            ce_buffer_printf(buffer, _G.allocator, "0x%llx%s:", key, sufix);
        }

        switch (type) {
            case CE_CDB_TYPE_SUBOBJECT: {
                uint64_t s = ce_cdb_a0->read_subobject(r, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "\n");
                _save_to_buffer(_db, buffer, s, level + 1);
            }
                break;

            case CE_CDB_TYPE_SET_SUBOBJECT: {
                ce_buffer_printf(buffer, _G.allocator, "\n");

                uint64_t num = ce_cdb_a0->read_objset_num(r, key);
                uint64_t objs[num];
                ce_cdb_a0->read_objset(r, key, objs);

                _push_space(buffer, level + 1);
                ce_buffer_printf(buffer, _G.allocator, "cdb_type: cdb_objset\n");

                for (int j = 0; j < num; ++j) {
                    uint64_t obj = objs[j];

                    const ce_cdb_uuid_t0 obj_uuid = ce_cdb_a0->obj_uid(_db, obj);
                    ce_uuid64_to_string(uuid_buffer, CE_ARRAY_LEN(uuid_buffer),
                                        (const ce_uuid64_t0 *) &obj_uuid);

                    _push_space(buffer, level + 1);
                    ce_buffer_printf(buffer, _G.allocator, "\"%s\":\n", uuid_buffer);

                    _save_to_buffer(_db, buffer, obj, level + 2);
                }
            }
                break;


            case CE_CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(r, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %f\n", f);
            }
                break;

            case CE_CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(r, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %s\n", s);
            }
                break;

            case CE_CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(r, key, 0);
                if (b) {
                    ce_buffer_printf(buffer, _G.allocator, " true\n");
                } else {
                    ce_buffer_printf(buffer, _G.allocator, " false\n");
                }
            }
                break;

            case CE_CDB_TYPE_NONE: {
                ce_buffer_printf(buffer, _G.allocator, " none\n");
            }
                break;

            case CE_CDB_TYPE_UINT64: {
                uint64_t i = ce_cdb_a0->read_uint64(r, key, 0);
                ce_buffer_printf(buffer, _G.allocator, " %llu\n", i);
            }
                break;

            case CE_CDB_TYPE_REF: {
                ce_cdb_uuid_t0 ref_uid = ce_cdb_a0->read_ref_uuid(r, key, (ce_cdb_uuid_t0) {0});

                ce_uuid64_to_string(uuid_buffer, CE_ARRAY_LEN(uuid_buffer),
                                    (const ce_uuid64_t0 *) &ref_uid);

                ce_buffer_printf(buffer, _G.allocator, " %s\n", uuid_buffer);
            }
                break;
//            case CE_CDB_TYPE_BLOB:{
//                uint64_t size = 0;
//                ce_cdb_a0->read_blob(reader, key, &size, NULL);
//                ce_buffer_printf(buffer, _G.allocator, " BLOB %llu\n", size);
//            }
                break;
            default:
                break;
        }
    }
}

static bool dump_str(ce_cdb_t0 _db,
                     const char *path,
                     uint64_t from,
                     uint64_t fs_root) {
    ce_vio_t0 *file = ce_fs_a0->open(fs_root, path, FS_OPEN_WRITE);

    if (!file) {
        return false;
    }

    char *buffer = NULL;

    _save_to_buffer(_db, &buffer, from, 0);
    file->vt->write(file->inst, buffer, ce_array_size(buffer), 1);
    ce_fs_a0->close(file);

    ce_array_free(buffer, _G.allocator);

    return true;
}

static struct ce_cdb_yaml_a0 ydb_api = {
        .load_from_file = load_obj,
        .load_to_nodes = cnodes_from_vio,
        .save_to_file = dump_str,
};

struct ce_cdb_yaml_a0 *ce_cdb_yaml_a0 = &ydb_api;

void CE_MODULE_LOAD(ydb)(struct ce_api_a0 *api,
                         int reload) {
    CE_UNUSED(reload);
    _G = (struct _G) {.allocator = ce_memory_a0->system};

    api->add_api(CE_YDB_A0_STR, &ydb_api, sizeof(ydb_api));
}

void CE_MODULE_UNLOAD(ydb)(struct ce_api_a0 *api,
                           int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
