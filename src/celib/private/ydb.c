#include <celib/api_system.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/hashlib.h>
#include <celib/log.h>
#include <celib/os.h>
#include <celib/ydb.h>
#include <celib/fs.h>


#include <celib/fmath.inl>
#include <celib/hash.inl>
#include <celib/buffer.inl>
#include <yaml/yaml.h>
#include <celib/cdb.h>

#define _G ydb_global
#define LOG_WHERE "ydb"

enum node_type {
    NODE_INVALID = 0,
    NODE_FLOAT = 1,
    NODE_STRING = 2,
    NODE_TRUE = 3,
    NODE_FALSE = 4,
    NODE_MAP = 5,
    NODE_SEQ = 6,
};

static struct _G {
    struct ce_hash_t obj_cache_map;
    struct ce_spinlock cache_lock;

    struct ce_hash_t modified_files_set;

    struct ce_alloc *allocator;
} _G;

static const char *get_key(uint64_t hash) {
    return ce_id_a0->str_from_id64(hash);
}

struct node_value {
    union {
        float f;
        char *string;
        uint32_t node_count;
    };
};


static uint64_t calc_key(const char *key) {
    return ce_id_a0->id64(key);
}

static void type_value_from_scalar(const uint8_t *scalar,
                                   enum node_type *type,
                                   struct node_value *vallue,
                                   bool is_key) {
    const char *scalar_str = (const char *) scalar;

    if (!strlen(scalar_str)) {
        *type = NODE_STRING;
        *vallue = (struct node_value) {.string = strdup(scalar_str)};
        return;
    }

    float f;

    if (!is_key) {
        if (sscanf(scalar_str, "%f", &f)) {
            *type = NODE_FLOAT;
            *vallue = (struct node_value) {.f = f};
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

    *type = NODE_STRING;
    *vallue = (struct node_value) {.string = strdup(scalar_str)};
}


uint64_t cdb_from_vio(struct ce_vio *vio,
                      struct ce_alloc *alloc) {
    struct parent_stack_state {
        enum node_type type;
        uint32_t node_count;

        uint64_t key_hash;
        uint64_t str_hash;

        uint64_t root_object;
        ce_cdb_obj_o *writer;

        char **str_array;
        float *float_array;
    };

    struct parent_stack_state *parent_stack = NULL;
    uint32_t parent_stack_top;
    uint64_t key = 0;

    uint64_t root_object = 0;
    ce_array_push(parent_stack,
                  ((struct parent_stack_state) {}),
                  _G.allocator);

    uint8_t *source_data = CE_ALLOC(alloc, uint8_t, vio->size(vio) + 1);
    memset(source_data, 0, vio->size(vio) + 1);
    vio->read(vio, source_data, sizeof(char), vio->size(vio));

    yaml_parser_t parser;
    if (!yaml_parser_initialize(&parser)) {
        ce_log_a0->error(LOG_WHERE, "Failed to initialize parser");
        goto error;
    }

    yaml_parser_set_input_string(&parser, source_data, vio->size(vio));

#define IS_KEY() (parent_stack[parent_stack_top].type == NODE_MAP)
#define HAS_KEY() (parent_stack[parent_stack_top].type == NODE_STRING)
    yaml_event_t event;

    do {
        parent_stack_top = ce_array_size(parent_stack) - 1;

        if (!yaml_parser_parse(&parser, &event)) {
            ce_log_a0->error(LOG_WHERE, "Parser error %d\n", parser.error);
            goto error;
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
                state = (struct parent_stack_state) {
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

            case YAML_SEQUENCE_END_EVENT: {
                struct parent_stack_state *s = &parent_stack[parent_stack_top];

                struct parent_stack_state *sm;
                sm = &parent_stack[parent_stack_top - 1];

                key = s->key_hash;

                const uint32_t float_len = ce_array_size(s->float_array);
                if (float_len) {
                    switch (s->node_count) {
                        case 3:
                            ce_cdb_a0->set_vec3(sm->writer, key,
                                                s->float_array);
                            break;
                        case 4:
                            ce_cdb_a0->set_vec4(sm->writer, key,
                                                s->float_array);
                            break;

                        case 16:
                            ce_cdb_a0->set_mat4(sm->writer, key,
                                                s->float_array);
                            break;
                    }

                    ce_array_free(s->float_array, alloc);
                }

                const uint32_t len = ce_array_size(s->str_array);
                if (len) {
                    uint64_t array = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                              0);
                    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(array);
                    for (int i = 0; i < len; ++i) {
                        const char *str = s->str_array[i];
                        ce_cdb_a0->set_str(w, ce_id_a0->id64(str), str);
                    }
                    ce_cdb_a0->write_commit(w);

                    ce_cdb_a0->set_subobject(sm->writer, key, array);

                    ce_array_free(s->str_array, alloc);
                }

                ce_array_pop_back(parent_stack);
                break;
            }

            case YAML_MAPPING_START_EVENT: {
                key = parent_stack[parent_stack_top].str_hash;

                uint64_t obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

                if (!root_object) {
                    root_object = obj;
                }

                state = (struct parent_stack_state) {
                        .type = NODE_MAP,
                        .root_object = obj,
                        .key_hash = key,
                        .writer = ce_cdb_a0->write_begin(obj)
                };


                ++parent_stack[parent_stack_top].node_count;

                if (HAS_KEY()) {
                    state.key_hash = parent_stack[parent_stack_top].str_hash;
                    ce_cdb_a0->set_subobjectw(
                            parent_stack[parent_stack_top - 1].writer,
                            state.key_hash, state.writer);
                    ce_array_pop_back(parent_stack);
                }

                ce_array_push(parent_stack, state, _G.allocator);
                break;
            }


            case YAML_MAPPING_END_EVENT: {
                struct parent_stack_state *s = &parent_stack[parent_stack_top];
                ce_cdb_a0->write_commit(s->writer);
                ce_array_pop_back(parent_stack);
                break;
            }

            case YAML_ALIAS_EVENT:
                break;

            case YAML_SCALAR_EVENT: {
                enum node_type type;
                struct node_value value;
                type_value_from_scalar(event.data.scalar.value,
                                       &type, &value, IS_KEY());


                if (IS_KEY()) {
                    uint64_t key_hash = ce_id_a0->id64(value.string);

                    state = (struct parent_stack_state) {.type = NODE_STRING, .str_hash = key_hash, .key_hash=key_hash};

                    ++parent_stack[parent_stack_top].node_count;
                    ce_array_push(parent_stack, state, _G.allocator);


                    // VALUE_WITH_KEY
                } else if (parent_stack[parent_stack_top].type == NODE_STRING) {
                    key = parent_stack[parent_stack_top].str_hash;
                    ce_cdb_obj_o *w = parent_stack[parent_stack_top - 1].writer;

                    switch (type) {
                        case NODE_FLOAT:
                            ce_cdb_a0->set_float(w, key, value.f);
                            break;

                        case NODE_STRING:
                            ce_cdb_a0->set_str(w, key, value.string);
                            break;

                        case NODE_TRUE:
                            ce_cdb_a0->set_bool(w, key, true);
                            break;

                        case NODE_FALSE:
                            ce_cdb_a0->set_bool(w, key, false);
                            break;

                        case NODE_INVALID:
                        case NODE_MAP:
                        case NODE_SEQ:
                            break;
                    }
//                    if (PREFAB_KEY == parent_stack[parent_stack_top].str_hash) {
//                        ce_array_push(inst->parent_file, value.string,
//                                      _G.allocator);
//                    }

                    ce_array_pop_back(parent_stack);

                    // VALUE_IN_SEQ
                } else if (parent_stack[parent_stack_top].type == NODE_SEQ) {
                    struct parent_stack_state *s = &parent_stack[parent_stack_top];
                    switch (type) {
                        case NODE_FLOAT:
                            ce_array_push(s->float_array, value.f, alloc);
                            break;

                        case NODE_STRING:
                            ce_array_push(s->str_array, value.string, alloc);
                            break;

                        case NODE_INVALID:
                        case NODE_TRUE:
                        case NODE_FALSE:
                        case NODE_MAP:
                        case NODE_SEQ:
                            break;
                    }

                    ++s->node_count;
                }

            }
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        }

    } while (event.type != YAML_STREAM_END_EVENT);

    return root_object;

    error:
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);

    return 0;
}

void expire_document_in_cache(const char *path,
                              uint64_t path_key) {
    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);
}

uint64_t load_obj_to_cache(const char *path,
                           uint64_t path_key) {

    const uint64_t fs_root = ce_id_a0->id64("source");

    ce_log_a0->debug(LOG_WHERE, "Load file %s to cache", path);

    struct ce_vio *f = ce_fs_a0->open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return 0;
    }

    uint64_t obj = ce_ydb_a0->cdb_from_vio(f, _G.allocator);
    ce_fs_a0->close(f);

    if (!obj) {
        ce_log_a0->error(LOG_WHERE, "Could not load yaml to cdb parse file %s",
                         path);
        return 0;
    }

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    ce_hash_add(&_G.obj_cache_map, path_key, obj, _G.allocator);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    return obj;
}

uint64_t get_obj(const char *path) {
    uint64_t path_key = ce_id_a0->id64(path);

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    uint64_t obj = ce_hash_lookup(&_G.obj_cache_map, path_key, 0);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    if (!obj) {
        obj = load_obj_to_cache(path, path_key);
    }

    return obj;
};

void modified(const char *path) {
    uint64_t hash = ce_id_a0->id64(path);
    ce_hash_add(&_G.modified_files_set, hash, true, _G.allocator);
}

void unmodified(const char *path) {
    uint64_t hash = ce_id_a0->id64(path);
    ce_hash_remove(&_G.modified_files_set, hash);
}

void parent_files(const char *path,
                  const char ***files,
                  uint32_t *count) {
    *count = 0;

//    struct ce_yng_doc *d = get(path);
//
//    if (!d) {
//        *files = NULL;
//        *count = 0;
//        return;
//    }
//
//    d->parent_files(d, files, count);
}

//void check_fs() {
//    ce_alloc *alloc = ce_memory_a0->system;
//
//    static uint64_t root = CE_ID64_0("source", 0x921f1370045bad6eULL);
//
//    auto *wd_it = ce_fs_a0->event_begin(root);
//    const auto *wd_end = ce_fs_a0->event_end(root);
//
//    while (wd_it != wd_end) {
//        if (wd_it->type == CE_WATCHDOG_EVENT_FILE_MODIFIED) {
//            ce_wd_ev_file_write_end *ev = (ce_wd_ev_file_write_end *) wd_it;
//
//            char *path = NULL;
//            ce_os_path_a0->join(&path, alloc, 2, ev->dir, ev->filename);
//
//            uint64_t path_key = CE_ID64_0(path);
//
//            if (ce_hash_contain(&_G.document_cache_map, path_key)) {
//                ce_log_a0->debug(LOG_WHERE, "Reload cached file %s", path);
//
//                expire_document_in_cache(path, path_key);
//                load_to_cache(path, path_key);
//            }
//
//            ce_buffer_free(path, alloc);
//        }
//
//        wd_it = ce_fs_a0->event_next(wd_it);
//    }
//}


static void _indent(char **buffer,
                    int level) {
    for (int j = 0; j < level; ++j) {
        ce_buffer_printf(buffer, _G.allocator, "  ");
    }
}

static void dump_yaml(char **buffer,
                      uint64_t from,
                      uint32_t level) {
    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    uint64_t keys[prop_count];
    ce_cdb_a0->prop_keys(from, keys);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];

        const char *k = ce_id_a0->str_from_id64(key);

        if (!k) {
            continue;
        }

        if (!ce_cdb_a0->prop_exist_norecursive(from, key)) {
            continue;
        }

        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, key);

        switch (type) {
            case CDB_TYPE_SUBOBJECT: {
                uint64_t s = ce_cdb_a0->read_subobject(from, key, 0);
                char *b = NULL;
                dump_yaml(&b, s, level + 1);
                if (ce_buffer_size(b)) {
                    _indent(buffer, level);
                    ce_buffer_printf(buffer, _G.allocator, "%s:\n", k);
                    ce_buffer_printf(buffer, _G.allocator, "%s", b);
                }
            }
                break;

            case CDB_TYPE_FLOAT: {
                float f = ce_cdb_a0->read_float(from, key, 0);
                _indent(buffer, level);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                ce_buffer_printf(buffer, _G.allocator, " %f", f);
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;

            case CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(from, key, 0);
                _indent(buffer, level);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                ce_buffer_printf(buffer, _G.allocator, " \"%s\"", s);
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;
            case CDB_TYPE_VEC3: {
                float v[3] = {};
                ce_cdb_a0->read_vec3(from, key, v);
                _indent(buffer, level);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                ce_buffer_printf(buffer, _G.allocator, " [%f, %f, %f]", v[0],
                                 v[1], v[2]);
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;
            case CDB_TYPE_VEC4: {
                float v[4] = {};
                ce_cdb_a0->read_vec3(from, key, v);
                _indent(buffer, level);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                ce_buffer_printf(buffer, _G.allocator, " [%f, %f, %f, %f]",
                                 v[0], v[1], v[2], v[3]);
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;

            case CDB_TYPE_BOOL: {
                bool b = ce_cdb_a0->read_bool(from, key, 0);
                _indent(buffer, level);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                if (b) {
                    ce_buffer_printf(buffer, _G.allocator, "true");
                } else {
                    ce_buffer_printf(buffer, _G.allocator, "false");
                }
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;

            case CDB_TYPE_NONE: {
                _indent(buffer, level);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                ce_buffer_printf(buffer, _G.allocator, "none");
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;

            case CDB_TYPE_UINT64: {
                _indent(buffer, level);
                uint64_t i = ce_cdb_a0->read_uint64(from, key, 0);
                ce_buffer_printf(buffer, _G.allocator, "%s:", k);
                ce_buffer_printf(buffer, _G.allocator, "%llu", i);
                ce_buffer_printf(buffer, _G.allocator, "\n");
            }
                break;

            case CDB_TYPE_PTR:
            case CDB_TYPE_REF:
            case CDB_TYPE_MAT4:
            case CDB_TYPE_BLOB:
            default:
                break;
        }
    }
}

void save(const char *path) {
    struct ce_vio *f = ce_fs_a0->open(ce_id_a0->id64("source"), path,
                                      FS_OPEN_WRITE);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return;
    }

    uint64_t obj = get_obj(path);

    char *b = NULL;
    dump_yaml(&b, obj, 0);

    f->write(f, b, sizeof(char), ce_buffer_size(b));
    ce_buffer_free(b, ce_memory_a0->system);

    ce_fs_a0->close(f);
    unmodified(path);
}


void save_all_modified() {
//    for (int i = 0; i < _G.document_cache_map.n; ++i) {
//        if (!ce_hash_contain(&_G.modified_files_set,
//                             _G.document_cache_map.keys[i])) {
//            continue;
//        }
//
//        save(_G.document_path[_G.document_cache_map.values[i]]);
//        return;
//    }
}

static struct ce_ydb_a0 ydb_api = {
        .get_obj = get_obj,
        .parent_files = parent_files,
        .save = save,
        .save_all_modified = save_all_modified,

        .cdb_from_vio = cdb_from_vio,
        .get_key = get_key,
        .key = calc_key,
};

struct ce_ydb_a0 *ce_ydb_a0 = &ydb_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {.allocator = ce_memory_a0->system};

    api->register_api("ce_ydb_a0", &ydb_api);
}

static void _shutdown() {
//    for (int i = 0; i < ce_array_size(_G.document_cache); ++i) {
////        ce_ydb_a0->destroy(_G.document_cache[i]);
//    }

    ce_hash_free(&_G.modified_files_set, _G.allocator);
    ce_hash_free(&_G.obj_cache_map, _G.allocator);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        ydb,
        {

        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)