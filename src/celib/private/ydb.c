#include <celib/api.h>
#include <celib/config.h>
#include <celib/memory/memory.h>
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/id.h>
#include <celib/log.h>
#include <celib/ydb.h>
#include <celib/fs.h>
#include <celib/memory/allocator.h>
#include <celib/math/math.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/cdb.h>
#include <celib/os/thread.h>
#include <celib/os/vio.h>

#include <cetech/resource/resource.h>

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
        uint64_t *child;
    };
} node_value;


static struct _G {
    ce_alloc_t0 *allocator;
} _G;


static bool _is_ref(const char *str) {
    return (str[0] == '0') && (str[1] == 'x');
}

static uint64_t _uid_from_str(const char *str) {
    uint64_t v = strtoul(str, NULL, 0);
    return v;
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
        uint64_t uid = _uid_from_str(scalar_str);
        *type = NODE_REF;
        *vallue = (node_value) {.ui = uid};
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

    uint32_t cnode_idx;
} parent_stack_state;

static uint32_t _push_cnode(cnode_t **nodes,
                            cnode_t node) {
    uint32_t idx = ce_array_size(*nodes);
    ce_array_push(*nodes, node, ce_memory_a0->system);
    return idx;
}

uint64_t cnodes_from_vio(ce_vio_t0 *vio,
                         cnode_t **cnodes,
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

        return 0;
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

            return 0;
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

                uint32_t cnode_idx = _push_cnode(cnodes, (cnode_t) {
                        .type = CNODE_OBJ_BEGIN,
                });

                state = (parent_stack_state) {
                        .type = NODE_MAP,
                        .key_hash = key,
                        .cnode_idx = cnode_idx,
                };

                ++parent_stack[parent_stack_top].node_count;

                if (HAS_KEY()) {
                    state.key_hash = parent_stack[parent_stack_top].str_hash;
                    ce_array_pop_back(parent_stack);

                    (*cnodes)[cnode_idx].key = parent_stack[parent_stack_top].str_hash;
                }

                ce_array_push(parent_stack, state, _G.allocator);
                break;
            }


            case YAML_MAPPING_END_EVENT: {
                ce_array_pop_back(parent_stack);

                uint32_t cnode_idx = parent_stack[parent_stack_top].cnode_idx;
                cnode_t *cnode = &(*cnodes)[cnode_idx];

                if (cnode->type == CNODE_OBJSET) {
                    _push_cnode(cnodes, (cnode_t) {
                            .type = CNODE_OBJSET_END
                    });
                } else {
                    if (!cnode->obj.uid) {
                        cnode->obj.uid = ce_cdb_a0->gen_uid(ce_cdb_a0->db());
                    }
                    _push_cnode(cnodes, (cnode_t) {
                            .type = CNODE_OBJ_END,
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
                            _push_cnode(cnodes, (cnode_t) {
                                    .type = CNODE_FLOAT,
                                    .key = key,
                                    .value.f = value.f,
                            });

                            break;

                        case NODE_UINT:
                            _push_cnode(cnodes, (cnode_t) {
                                    .type = CNODE_UINT,
                                    .key = key,
                                    .value.uint64 = value.ui
                            });

                            break;

                        case NODE_STRING:
                            if (key == CDB_UID_PROP) {
                                uint32_t cnode_idx = parent_stack[parent_stack_top - 1].cnode_idx;
                                cnode_t *cnode = &(*cnodes)[cnode_idx];

                                uint64_t uid = _uid_from_str(value.string);
                                cnode->obj.uid = uid;
                            } else if (key == CDB_TYPE_PROP) {
                                uint32_t cnode_idx = parent_stack[parent_stack_top - 1].cnode_idx;
                                cnode_t *cnode = &(*cnodes)[cnode_idx];

                                uint64_t type = 0;

                                if (_is_ref(value.string)) {
                                    type = _uid_from_str(value.string);
                                } else {
                                    type = ce_id_a0->id64(value.string);
                                }

                                if (type == CDB_OBJSET) {
                                    cnode->type = CNODE_OBJSET;
                                } else {
                                    cnode->obj.type = type;
                                }

                            } else if (key == CDB_INSTANCE_PROP) {
                                uint32_t cnode_idx = parent_stack[parent_stack_top - 1].cnode_idx;
                                cnode_t *cnode = &(*cnodes)[cnode_idx];

                                uint64_t uid = _uid_from_str(value.string);
                                cnode->obj.instance_of = uid;
                            } else if (_is_ref(value.string)) {
                                uint64_t uid = _uid_from_str(value.string);

                                _push_cnode(cnodes, (cnode_t) {
                                        .type = CNODE_REF,
                                        .key = key,
                                        .value.ref = uid,
                                });

                            } else {
                                _push_cnode(cnodes, (cnode_t) {
                                        .type = CNODE_STRING,
                                        .key = key,
                                        .value.str = value.string
                                });
                            }

                            break;

                        case NODE_TRUE:
                            _push_cnode(cnodes, (cnode_t) {
                                    .type = CNODE_BOOL,
                                    .key = key,
                                    .value.b = true
                            });

                            break;

                        case NODE_FALSE:
                            _push_cnode(cnodes, (cnode_t) {
                                    .type = CNODE_BOOL,
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

    return (*cnodes)[0].obj.uid;
}

void _log_cnodes(cnode_t *cnodes) {
    uint32_t cnodes_n = ce_array_size(cnodes);
    for (int j = 0; j < cnodes_n; ++j) {
        cnode_t node = cnodes[j];

        switch (node.type) {
            default:
            case CNODE_INVALID: {
            }
                break;
            case CNODE_FLOAT: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %f",
                                 ce_id_a0->str_from_id64(node.key), node.value.f);
            }
                break;
            case CNODE_UINT: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %llu",
                                 ce_id_a0->str_from_id64(node.key), node.value.uint64);
            }
                break;
            case CNODE_REF: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %llx",
                                 ce_id_a0->str_from_id64(node.key), node.value.ref);
            }
                break;
            case CNODE_STRING: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %s",
                                 ce_id_a0->str_from_id64(node.key), node.value.str);
            }
                break;
            case CNODE_BOOL: {
                ce_log_a0->debug(LOG_WHERE, "\t%s: %s",
                                 ce_id_a0->str_from_id64(node.key),
                                 node.value.b ? "true" : "false");
            }
                break;
            case CNODE_OBJ_BEGIN: {
                ce_log_a0->debug(LOG_WHERE, "OBJ_BEGIN");

                if (node.key) {
                    ce_log_a0->debug(LOG_WHERE, "key: %s", ce_id_a0->str_from_id64(node.key));
                }

                ce_log_a0->debug(LOG_WHERE, "id: %llx", node.obj.uid);
                if (node.obj.instance_of) {
                    ce_log_a0->debug(LOG_WHERE, "inst: %llx", node.obj.instance_of);
                }
            }
                break;
            case CNODE_OBJ_END: {
                ce_log_a0->debug(LOG_WHERE, "OBJ_END");
            }
                break;
            case CNODE_OBJSET: {
                ce_log_a0->debug(LOG_WHERE, "\tOBJSET %s",
                                 ce_id_a0->str_from_id64(node.key));
            }
                break;
            case CNODE_OBJSET_END: {
                ce_log_a0->debug(LOG_WHERE, "\tOBJSET_END");
            }
                break;
        }
    }
}

void _create_root_obj(cnode_t *cnodes,
                      ce_cdb_t0 tmp_db) {

//    _log_cnodes(cnodes);

    struct state_t {
        uint32_t node_idx;
        cnode_e type;
        ce_cdb_obj_o0 *writer;
    } *states = NULL;

    uint32_t cnodes_n = ce_array_size(cnodes);
    for (int j = 0; j < cnodes_n; ++j) {
        cnode_t node = cnodes[j];

        uint32_t state_top = ce_array_size(states) - 1;

        switch (node.type) {
            default:
            case CNODE_INVALID: {
            }
                break;
            case CNODE_FLOAT: {
                struct state_t *state = &states[state_top];
                ce_cdb_a0->set_float(state->writer, node.key, node.value.f);
            }
                break;
            case CNODE_UINT: {
                struct state_t *state = &states[state_top];
                ce_cdb_a0->set_uint64(state->writer, node.key, node.value.uint64);
            }
                break;
            case CNODE_REF: {
                struct state_t *state = &states[state_top];
                ce_cdb_a0->set_ref(state->writer, node.key, node.value.ref);
            }
                break;
            case CNODE_STRING: {
                struct state_t *state = &states[state_top];
                ce_cdb_a0->set_str(state->writer, node.key, node.value.str);
            }
                break;
            case CNODE_BOOL: {
                struct state_t *state = &states[state_top];
                ce_cdb_a0->set_bool(state->writer, node.key, node.value.b);
            }
                break;
            case CNODE_OBJ_BEGIN: {
                uint64_t obj;
                uint64_t uid = node.obj.uid;
                if (!uid) {
                    uid = ce_cdb_a0->gen_uid(tmp_db);
                }

                if (node.obj.instance_of) {
                    obj = ce_cdb_a0->create_from_uid(tmp_db, node.obj.instance_of, uid);
                } else {
                    obj = ce_cdb_a0->create_object_uid(tmp_db, uid, node.obj.type, true);
                }

                struct state_t *state = &states[state_top];
                if (state->type == CNODE_OBJSET) {
                    struct state_t *parent_state = &states[state_top - 1];
                    ce_cdb_a0->objset_add_obj(parent_state->writer,
                                              cnodes[state->node_idx].key, obj);
                } else if (node.key) {
                    ce_cdb_a0->set_subobject(state->writer, node.key, obj);
                }

                struct state_t new_state = {
                        .type = CNODE_OBJ_BEGIN,
                        .writer = ce_cdb_a0->write_begin(tmp_db, obj),
                        .node_idx = j,
                };
                ce_array_push(states, new_state, _G.allocator);
            }
                break;
            case CNODE_OBJ_END: {
                struct state_t *state = &states[state_top];

                ce_cdb_a0->write_commit(state->writer);
                ce_array_pop_back(states);
            }
                break;
            case CNODE_OBJSET: {
                struct state_t state = {
                        .type = CNODE_OBJSET,
                        .node_idx = j,
                };

                ce_array_push(states, state, _G.allocator);
            }
                break;
            case CNODE_OBJSET_END: {
                ce_array_pop_back(states);
            }
                break;
        }
    }
}

uint64_t load_obj(const char *path,
                  cnode_t **cnodes,
                  uint64_t path_key) {

    const uint64_t fs_root = ce_id_a0->id64("source");

    ce_log_a0->debug(LOG_WHERE, "Load file %s", path);

    ce_vio_t0 *f = ce_fs_a0->open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return 0;
    }

    uint64_t obj = cnodes_from_vio(f, cnodes, _G.allocator);
    ce_fs_a0->close(f);

    if (!obj) {
        ce_log_a0->error(LOG_WHERE, "Could not load file %s", path);
        return 0;
    }

    return obj;
}

uint64_t get_obj(const char *path) {
    uint64_t path_key = ce_id_a0->id64(path);

    cnode_t *cnodes = NULL;
    uint64_t obj = load_obj(path, &cnodes, path_key);
    _create_root_obj(cnodes, ce_cdb_a0->db());

    ce_array_free(cnodes, _G.allocator);
    return obj;
}

void read_cnodes(const char *path,
                 cnode_t **cnodes) {
    uint64_t path_key = ce_id_a0->id64(path);
    load_obj(path, cnodes, path_key);
}

void _dump_cnodes(ce_cdb_t0 db,
                  cnode_t *cnodes,
                  char **outputs) {

    char *str_buffer = NULL;
    char *blob_buffer = NULL;
    cnode_t *nodes = NULL;

    uint64_t n = ce_array_size(cnodes);
    for (int i = 0; i < n; ++i) {
        cnode_t node = cnodes[i];

        switch (node.type) {
            default:
            case CNODE_INVALID: {
            }
                break;

            case CNODE_FLOAT:
            case CNODE_UINT:
            case CNODE_REF:
            case CNODE_BOOL:
            case CNODE_OBJSET:
            case CNODE_OBJSET_END:
            case CNODE_OBJ_BEGIN:
            case CNODE_OBJ_END: {
                ce_array_push(nodes, node, _G.allocator);
            }
                break;

            case CNODE_STRING: {
                uint64_t stroffset = ce_array_size(str_buffer);

                if (node.value.str) {
                    ce_array_push_n(str_buffer,
                                    node.value.str, strlen(node.value.str) + 1, _G.allocator);
                }

                node.value.uint64 = stroffset;
                ce_array_push(nodes, node, _G.allocator);

            }
                break;
            case CNODE_BLOB: {
                uint64_t bloboffset = ce_array_size(blob_buffer);

                if (node.blob.data) {
                    ce_array_push_n(blob_buffer, node.blob.data, node.blob.size, _G.allocator);
                }

                node.value.uint64 = bloboffset;

                ce_array_push(nodes, node, _G.allocator);

            }
                break;
        }
    }

    cdb_binobj_header header = {
            .version = 0,
            .blob_buffer_size = ce_array_size(blob_buffer),
            .string_buffer_size = ce_array_size(str_buffer),
            .node_count = ce_array_size(nodes),
    };

    ce_array_push_n(*outputs, (char *) &header, sizeof(cdb_binobj_header), _G.allocator);
    ce_array_push_n(*outputs, (char *) nodes, sizeof(cnode_t) * header.node_count, _G.allocator);
    ce_array_push_n(*outputs, (char *) str_buffer, header.string_buffer_size, _G.allocator);
    ce_array_push_n(*outputs, (char *) blob_buffer, header.blob_buffer_size, _G.allocator);
}

static struct ce_ydb_a0 ydb_api = {
        .get_obj = get_obj,
        .read_cnodes = read_cnodes,
        .create_root_obj = _create_root_obj,
        .dump_cnodes = _dump_cnodes,
        .cnodes_from_vio = cnodes_from_vio,
};

struct ce_ydb_a0 *ce_ydb_a0 = &ydb_api;

void CE_MODULE_LOAD(ydb)(struct ce_api_a0 *api,
                         int reload) {
    CE_UNUSED(reload);
    _G = (struct _G) {.allocator = ce_memory_a0->system};

    api->add_api(CE_YDB_API, &ydb_api, sizeof(ydb_api));
}

void CE_MODULE_UNLOAD(ydb)(struct ce_api_a0 *api,
                           int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _G = (struct _G) {};
}
