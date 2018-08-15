// TODO: string table

#include <celib/api_system.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/module.h>
#include <celib/yng.h>
#include <celib/hashlib.h>
#include <celib/log.h>
#include <celib/os.h>
#include <celib/array.inl>
#include <celib/hash.inl>
#include <celib/murmur_hash.inl>

#include "include/yaml/yaml.h"

#define _G yamlng_global
#define LOG_WHERE "yamlng"

static struct _G {
    struct ce_hash_t key_to_str;
    uint32_t *key_to_str_offset;
    char *key_to_str_data;
    struct ce_spinlock key_lock;
    struct ce_alloc *allocator;
} _G;


static void add_key(const char *key,
             uint32_t key_len,
             uint64_t key_hash) {
    ce_os_a0->thread->spin_lock(&_G.key_lock);
    const uint32_t idx = ce_array_size(_G.key_to_str_offset);
    const uint32_t offset = ce_array_size(_G.key_to_str_data);

    ce_array_push_n(_G.key_to_str_data, key, sizeof(char) * (key_len + 1),
                    _G.allocator);
    ce_array_push(_G.key_to_str_offset, offset, _G.allocator);

    ce_hash_add(&_G.key_to_str, key_hash, idx, _G.allocator);
    ce_os_a0->thread->spin_unlock(&_G.key_lock);
}

static const char *get_key(uint64_t hash) {
    uint32_t idx = ce_hash_lookup(&_G.key_to_str, hash, UINT32_MAX);
    if (UINT32_MAX == idx) {
        return NULL;
    }

    return &_G.key_to_str_data[_G.key_to_str_offset[idx]];
}

struct node_value {
    union {
        float f;
        char *string;
        uint32_t node_count;
    };
};

struct doc_inst {
    struct ce_alloc *alloc;
    struct ce_yng_doc *doc;

    char **parent_file;

    struct ce_hash_t key_map;
    enum node_type *type;
    uint64_t *hash;
    struct node_value *value;
    uint32_t *first_child;
    uint32_t *next_sibling;
    uint32_t *parent;

    bool modified;
};


static uint64_t hash_combine(uint64_t lhs,
                      uint64_t rhs) {
    if (lhs == 0) return rhs;
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

static uint64_t calc_key(const char *key) {
    uint64_t hash = 0;
    bool parse = false;

    char *begin;
    char *it = (char *) key;
    while (*it != '\0') {
        if (!parse) {
            begin = it;
            parse = true;
        } else if (*it == '.') {
            const uint32_t size = it - begin;
            const uint64_t part_hash = ce_hash_murmur2_64(begin,
                                                          size,
                                                          0);
            add_key(begin, size, part_hash);
            hash = hash_combine(hash, part_hash);
            parse = false;
        }

        ++it;
    }

    const uint32_t size = it - begin;
    const uint64_t part_hash = ce_hash_murmur2_64(begin, size, 0);
    add_key(begin, size, part_hash);
    hash = hash_combine(hash, part_hash);

    return hash;
}

static uint64_t combine_key(const uint64_t *keys,
                     uint32_t count) {
    uint64_t hash = keys[0];

    for (uint32_t i = 1; i < count; ++i) {
        hash = hash_combine(hash, keys[i]);
    }

    return hash;
}


static uint64_t combine_key_str(const char **keys,
                         uint32_t count) {
    uint64_t hash = ce_id_a0->id64(keys[0]);

    for (uint32_t i = 1; i < count; ++i) {
        hash = hash_combine(hash, ce_id_a0->id64(keys[i]));
    }

    return hash;
}

static uint32_t new_node(struct ce_yng_doc *doc,
                  enum node_type type,
                  struct node_value value,
                  uint32_t parent,
                  uint64_t hash) {

    struct doc_inst *inst = (struct doc_inst *) doc->inst;

    const uint32_t idx = ce_array_size(inst->type);

    ce_array_push(inst->type, type, _G.allocator);
    ce_array_push(inst->hash, hash, _G.allocator);
    ce_array_push(inst->value, value, _G.allocator);
    ce_array_push(inst->first_child, 0, _G.allocator);
    ce_array_push(inst->next_sibling, 0, _G.allocator);
    ce_array_push(inst->parent, parent, _G.allocator);

    if (parent) {
        inst->parent[idx] = parent;

        if (inst->first_child[parent] == 0) {
            inst->first_child[parent] = idx;
        } else {
            uint32_t first_child_idx = inst->first_child[parent];
            inst->first_child[parent] = idx;
            inst->next_sibling[idx] = first_child_idx;
        }
    }

    return idx;
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

static bool has_key(struct ce_yng_doc *_inst,
             uint64_t key) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;
    return ce_hash_contain(&inst->key_map, key);
}


static struct ce_yng_node get(struct ce_yng_doc *_inst,
                          uint64_t key) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;
    return (struct ce_yng_node) {.idx =  (uint32_t) ce_hash_lookup(
            &inst->key_map, key, 0), .d = inst->doc};
}


static struct ce_yng_node get_seq(struct ce_yng_doc *_inst,
                              uint64_t key,
                              uint32_t idx) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    uint32_t n_idx = ce_hash_lookup(&inst->key_map, key, (uint32_t) 0);
    uint32_t size = inst->value[n_idx].node_count;

    uint32_t it = inst->first_child[n_idx];
    for (uint32_t i = 1; i < size - idx; ++i) {
        it = inst->next_sibling[it];
    }

    return (struct ce_yng_node) {.idx = it, .d = inst->doc};
}

static enum node_type type(struct ce_yng_doc *_inst,
                    struct ce_yng_node node) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;
    return inst->type[node.idx];
}

static uint64_t get_hash(struct ce_yng_doc *_inst,
                  struct ce_yng_node node) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;
    return inst->hash[node.idx];
}

static uint32_t get_size(struct ce_yng_doc *_inst,
                  struct ce_yng_node node) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;
    return inst->value[node.idx].node_count;
}

static const char *as_string(struct ce_yng_doc *_inst,
                      struct ce_yng_node node,
                      const char *defaultt) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    if (!node.idx) {
        return defaultt;
    }

    return inst->value[node.idx].string;
}

static float as_float(struct ce_yng_doc *_inst,
               struct ce_yng_node node,
               float defaultt) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    if (!node.idx) {
        return defaultt;
    }


    return inst->value[node.idx].f;
}

static bool as_bool(struct ce_yng_doc *_inst,
             struct ce_yng_node node,
             bool defaultt) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    if (!node.idx) {
        return defaultt;
    }


    return inst->type[node.idx] == NODE_TRUE;
}

static void as_vec3(struct ce_yng_doc *_inst,
             struct ce_yng_node node,
             float *value) {

    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    const uint32_t count = inst->value[node.idx].node_count;
    if (3 != count) {
        return;
    }

    uint32_t it = inst->first_child[node.idx];
    value[count - 1] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 2] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 3] = inst->value[it].f;
}

static void as_vec4(struct ce_yng_doc *_inst,
             struct ce_yng_node node,
             float *value) {

    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    const uint32_t count = inst->value[node.idx].node_count;
    if (4 != count) {
        return;
    }

    uint32_t it = inst->first_child[node.idx];
    value[count - 1] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 2] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 3] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 4] = inst->value[it].f;
}

static void as_mat4(struct ce_yng_doc *_inst,
             struct ce_yng_node node,
             float *value) {

    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    const uint32_t count = inst->value[node.idx].node_count;
    if (16 != count) {
        return;
    }

    uint32_t it = inst->first_child[node.idx];
    value[count - 1] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 2] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 3] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 4] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 5] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 6] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 7] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 8] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 9] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 10] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 11] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 12] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 13] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 14] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 15] = inst->value[it].f;

    it = inst->next_sibling[it];
    value[count - 16] = inst->value[it].f;
}

static const char *get_string(struct ce_yng_doc *_inst,
                       uint64_t key,
                       const char *defaultt) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    struct ce_yng_node node = {.idx = (uint32_t) ce_hash_lookup(
            &inst->key_map, key, 0)};
    return as_string(_inst, node, defaultt);
}

static float get_float(struct ce_yng_doc *_inst,
                uint64_t key,
                float defaultt) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    struct ce_yng_node node = {.idx = (uint32_t) ce_hash_lookup(
            &inst->key_map, key, 0)};
    return as_float(_inst, node, defaultt);
}

static bool get_bool(struct ce_yng_doc *_inst,
              uint64_t key,
              bool defaultt) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    struct ce_yng_node node = {.idx = (uint32_t) ce_hash_lookup(
            &inst->key_map, key, 0)};
    return as_bool(_inst, node, defaultt);
}

static void set_float(struct ce_yng_doc *_inst,
               struct ce_yng_node node,
               float value) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;
    inst->value[node.idx].f = value;
    inst->modified = true;
}

static void set_bool(struct ce_yng_doc *_inst,
              struct ce_yng_node node,
              bool value) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    inst->type[node.idx] = value ? NODE_TRUE : NODE_FALSE;
    inst->modified = true;
}


static void set_string(struct ce_yng_doc *_inst,
                struct ce_yng_node node,
                const char *value) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    char *str = ce_memory_a0->str_dup(value, _G.allocator);
    inst->value[node.idx].string = str;
    inst->modified = true;
}

static void set_vec3(struct ce_yng_doc *_inst,
              struct ce_yng_node node,
              float *value) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    uint32_t it = inst->first_child[node.idx];
    inst->value[it].f = value[2];

    it = inst->next_sibling[it];
    inst->value[it].f = value[1];

    it = inst->next_sibling[it];
    inst->value[it].f = value[0];

    inst->modified = true;
}

static void set_vec4(struct ce_yng_doc *_inst,
              struct ce_yng_node node,
              float *value) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    uint32_t it = inst->first_child[node.idx];
    inst->value[it].f = value[3];

    it = inst->next_sibling[it];
    inst->value[it].f = value[2];

    it = inst->next_sibling[it];
    inst->value[it].f = value[1];

    it = inst->next_sibling[it];
    inst->value[it].f = value[0];
    inst->modified = true;
}

static void set_mat4(struct ce_yng_doc *_inst,
              struct ce_yng_node node,
              float *value) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    uint32_t it = inst->first_child[node.idx];
    inst->value[it].f = value[15];

    it = inst->next_sibling[it];
    inst->value[it].f = value[14];

    it = inst->next_sibling[it];
    inst->value[it].f = value[13];

    it = inst->next_sibling[it];
    inst->value[it].f = value[12];

    it = inst->next_sibling[it];
    inst->value[it].f = value[11];

    it = inst->next_sibling[it];
    inst->value[it].f = value[10];

    it = inst->next_sibling[it];
    inst->value[it].f = value[9];

    it = inst->next_sibling[it];
    inst->value[it].f = value[8];

    it = inst->next_sibling[it];
    inst->value[it].f = value[7];

    it = inst->next_sibling[it];
    inst->value[it].f = value[6];

    it = inst->next_sibling[it];
    inst->value[it].f = value[5];

    it = inst->next_sibling[it];
    inst->value[it].f = value[4];

    it = inst->next_sibling[it];
    inst->value[it].f = value[3];

    it = inst->next_sibling[it];
    inst->value[it].f = value[2];

    it = inst->next_sibling[it];
    inst->value[it].f = value[1];

    it = inst->next_sibling[it];
    inst->value[it].f = value[0];

    inst->modified = true;
}

static void foreach_dict_node(struct ce_yng_doc *_inst,
                       struct ce_yng_node node,
                       ce_yng_foreach_map_t foreach_clb,
                       void *data) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    uint32_t it = inst->first_child[node.idx];
    while (0 != it) {
        struct ce_yng_node key = {.idx=it, .d = inst->doc};
        struct ce_yng_node value = {.idx=inst->first_child[it], .d = inst->doc};

        foreach_clb(key, value, data);

        it = inst->next_sibling[it];
    }
}

static void foreach_seq_node(struct ce_yng_doc *_inst,
                      struct ce_yng_node node,
                      ce_yng_foreach_seq_t foreach_clb,
                      void *data) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    uint32_t idx = 0;
    uint32_t it = inst->first_child[node.idx];
    while (0 != it) {
        struct ce_yng_node value = {.idx=it, .d = inst->doc};

        foreach_clb(idx, value, data);

        it = inst->next_sibling[it];
        ++idx;
    }
}


bool parse_yaml(struct ce_alloc *alloc,
                struct ce_vio *vio,
                struct doc_inst *inst) {
    yaml_parser_t parser;
    yaml_event_t event;

    struct parent_stack_state {
        enum node_type type;
        uint32_t idx;
        uint32_t node_count;
        uint64_t key_hash;
        uint64_t str_hash;
    };

    struct parent_stack_state *parent_stack = NULL;
    uint32_t parent_stack_top;
    uint32_t tmp_idx;
    uint64_t key = 0;

    ce_array_push(parent_stack, (struct parent_stack_state) {}, _G.allocator);

    uint8_t *source_data = CE_ALLOC(alloc, uint8_t, vio->size(vio) + 1);
    memset(source_data, 0, vio->size(vio) + 1);
    vio->read(vio, source_data, sizeof(char), vio->size(vio));

    if (!yaml_parser_initialize(&parser)) {
        ce_log_a0->error(LOG_WHERE, "Failed to initialize parser");
        goto error;
    }

    yaml_parser_set_input_string(&parser, source_data,
                                 vio->size(vio));

#define IS_KEY() (parent_stack[parent_stack_top].type == NODE_MAP)
#define HAS_KEY() (parent_stack[parent_stack_top].type == NODE_STRING)
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

            case YAML_SEQUENCE_START_EVENT:
                key = parent_stack[parent_stack_top].key_hash;

                tmp_idx = new_node(inst->doc,
                                   NODE_SEQ,
                                   (struct node_value) {},
                                   parent_stack[parent_stack_top].idx, key);

                ce_hash_add(&inst->key_map, key, tmp_idx, _G.allocator);

                if (HAS_KEY()) {
                    ce_array_pop_back(parent_stack);
                }

                ++parent_stack[parent_stack_top].node_count;

                state = (struct parent_stack_state) {.idx = tmp_idx, .type = NODE_SEQ, .key_hash = key};

                ce_array_push(parent_stack, state, _G.allocator);
                break;

            case YAML_SEQUENCE_END_EVENT:
                inst->value[parent_stack[parent_stack_top].idx].node_count = parent_stack[parent_stack_top].node_count;
                ce_array_pop_back(parent_stack);
                break;

            case YAML_MAPPING_START_EVENT:
                key = parent_stack[parent_stack_top].key_hash;

                if (parent_stack[parent_stack_top].type == NODE_SEQ) {
                    key = parent_stack[parent_stack_top].key_hash;

                    char buffer[32] = {};
                    sprintf(buffer, "%d",
                            parent_stack[parent_stack_top].node_count);

                    key = hash_combine(key, calc_key(buffer));
                }

                tmp_idx = new_node(inst->doc, NODE_MAP, (struct node_value) {},
                                   parent_stack[parent_stack_top].idx, key);

                ce_hash_add(&inst->key_map, key, tmp_idx, _G.allocator);

                if (HAS_KEY()) {
                    ce_array_pop_back(parent_stack);
                }

                state = (struct parent_stack_state) {.idx = tmp_idx, .type = NODE_MAP, .key_hash = key};

                ++parent_stack[parent_stack_top].node_count;
                ce_array_push(parent_stack, state, _G.allocator);
                break;

            case YAML_MAPPING_END_EVENT:
                inst->value[parent_stack[parent_stack_top].idx].node_count = parent_stack[parent_stack_top].node_count;
                ce_array_pop_back(parent_stack);
                break;

            case YAML_ALIAS_EVENT:
                break;

            case YAML_SCALAR_EVENT: {
                enum node_type type;
                struct node_value value;
                type_value_from_scalar(event.data.scalar.value,
                                       &type, &value, IS_KEY());

                const uint64_t PREFAB_KEY = ce_id_a0->id64("PREFAB");

                if (IS_KEY()) {
                    uint64_t key_hash = ce_id_a0->id64(value.string);
                    uint64_t parent_key = parent_stack[parent_stack_top].key_hash;
                    key = hash_combine(parent_key, key_hash);

                    uint32_t tmp_idx = new_node(inst->doc, type, value,
                                                parent_stack[parent_stack_top].idx,
                                                key);

                    state = (struct parent_stack_state) {.idx = tmp_idx, .type = NODE_STRING, .key_hash = key, .str_hash = key_hash};

                    ++parent_stack[parent_stack_top].node_count;
                    ce_array_push(parent_stack, state, _G.allocator);

                    // VALUE_WITH_KEY
                } else if (parent_stack[parent_stack_top].type == NODE_STRING) {
                    key = parent_stack[parent_stack_top].key_hash;

                    if (PREFAB_KEY == parent_stack[parent_stack_top].str_hash) {
                        ce_array_push(inst->parent_file, value.string,
                                      _G.allocator);
                    }

                    uint32_t tmp_idx = new_node(inst->doc, type, value,
                                                parent_stack[parent_stack_top].idx,
                                                key);
                    ce_hash_add(&inst->key_map, key, tmp_idx, _G.allocator);

                    ce_array_pop_back(parent_stack);

                    // VALUE_IN_SEQ
                } else if (parent_stack[parent_stack_top].type == NODE_SEQ) {
                    key = parent_stack[parent_stack_top].key_hash;

                    char buffer[32] = {};
                    sprintf(buffer, "%d",
                            parent_stack[parent_stack_top].node_count);

                    key = hash_combine(key, calc_key(buffer));

                    uint32_t tmp_idx = new_node(inst->doc, type, value,
                                                parent_stack[parent_stack_top].idx,
                                                key);
                    ce_hash_add(&inst->key_map, key, tmp_idx, _G.allocator);

                    ++parent_stack[parent_stack_top].node_count;
                }

            }
                break;
        }

        if (event.type != YAML_STREAM_END_EVENT) {
            yaml_event_delete(&event);
        }

    } while (event.type != YAML_STREAM_END_EVENT);

    return true;

    error:
    yaml_event_delete(&event);
    yaml_parser_delete(&parser);

    return false;
}

static void save_recursive(struct doc_inst *inst,
                    uint32_t root,
                    yaml_emitter_t *emitter) {
    enum node_type type = inst->type[root];
    struct node_value value = inst->value[root];
    char buffer[128];

    yaml_event_t event;

    switch (type) {
        case NODE_FLOAT:
            sprintf(buffer, "%f", value.f);
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *) buffer,
                                         strlen(buffer),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_STRING:
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *) value.string,
                                         strlen(value.string),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_TRUE:
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *) "yes", strlen("yes"),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);

            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_FALSE:
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *) "no", strlen("no"),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);

            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_MAP: {
            yaml_mapping_start_event_initialize(&event, NULL, NULL, 1,
                                                YAML_BLOCK_MAPPING_STYLE);

            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            uint32_t childs[value.node_count];

            uint32_t it = 0;
            it = inst->first_child[root];
            for (int j = 0; j < value.node_count; ++j) {
                childs[value.node_count - j - 1] = it;
                it = inst->next_sibling[it];
            }

            for (int j = 0; j < value.node_count; ++j) {
                it = childs[j];
                save_recursive(inst, it, emitter);

                uint32_t value_it = inst->first_child[it];
                save_recursive(inst, value_it, emitter);
            }

            yaml_mapping_end_event_initialize(&event);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;
        }
            break;

        case NODE_SEQ: {
            yaml_sequence_start_event_initialize(&event, NULL, NULL, 1,
                                                 YAML_FLOW_SEQUENCE_STYLE);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            uint32_t childs[value.node_count];

            uint32_t it = 0;
            it = inst->first_child[root];
            for (int j = 0; j < value.node_count; ++j) {
                childs[value.node_count - j - 1] = it;
                it = inst->next_sibling[it];
            }

            for (int j = 0; j < value.node_count; ++j) {
                it = childs[j];
                save_recursive(inst, it, emitter);
            }


            yaml_sequence_end_event_initialize(&event);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;

        }
            break;

        case NODE_INVALID:
            break;
    }

    inst->modified = false;
    return;

    error:
    return;
}

static int write_handler(void *ext,
                  unsigned char *buffer,
                  size_t size) {
    struct ce_vio *output = (struct ce_vio *) ext;
    output->write(output, buffer, sizeof(unsigned char), size);
    return 1;
}

static bool save_yaml(struct ce_alloc *alloc,
               struct ce_vio *vio,
               struct ce_yng_doc *doc) {
//    unsigned char kim_nuke_shit[4096];

    yaml_emitter_t emitter;
    yaml_event_t event;

    struct ce_yng_doc *d = doc;
    struct doc_inst *inst = (struct doc_inst *) (d->inst);
    struct ce_yng_node root_node = d->get(d, 0);


    if (!yaml_emitter_initialize(&emitter)) {
        ce_log_a0->error(LOG_WHERE, "Could not initialize YAML emiter.");
        return false;
    }

    yaml_emitter_set_output(&emitter, write_handler, vio);

    // BEGIN STREAM
    yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    // BODY
    save_recursive(inst, root_node.idx, &emitter);

    // END DOC
    yaml_document_end_event_initialize(&event, 1);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    // END STREAM
    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event))
        goto error;

    return true;

    error:
    ce_log_a0->error(LOG_WHERE, "Error emmit");
    yaml_emitter_delete(&emitter);

    return false;
}

static struct ce_yng_node create_tree(struct ce_yng_doc *_inst,
                                  const char **keys,
                                  uint32_t keys_count) {
    uint32_t first_nonexist = 0;
    uint64_t last_exist_key = 0;
    for (int i = 0; i < keys_count; ++i) {
        uint64_t key = combine_key_str(keys, i + 1);
        if (!has_key(_inst, key)) {
            first_nonexist = i;
            break;
        } else {
            last_exist_key = key;
        }
    }

    struct ce_yng_node n = get(_inst, last_exist_key);
    struct doc_inst *inst = (struct doc_inst *) (_inst->inst);

    uint32_t parent = n.idx;
    for (int i = first_nonexist; i < keys_count - 1; ++i) {
        uint64_t key = combine_key_str(keys, i + 1);
        inst->value[parent].node_count += 1;

        uint32_t key_idx = new_node(
                inst->doc,
                NODE_STRING,
                (struct node_value) {.string = ce_memory_a0->str_dup(keys[i],
                                                                     _G.allocator)},
                parent, key);

        uint32_t new_map_idx = new_node(
                inst->doc,
                NODE_MAP, (struct node_value) {.node_count=0},
                key_idx, key);

        ce_hash_add(&inst->key_map, key, new_map_idx, _G.allocator);

        parent = new_map_idx;
    }

    uint64_t key_hash = ce_id_a0->id64(keys[keys_count - 1]);
    inst->value[parent].node_count += 1;
    uint32_t key_idx = new_node(
            inst->doc,
            NODE_STRING,
            (struct node_value) {.string = ce_memory_a0->str_dup(
                    keys[keys_count - 1],
                    _G.allocator)},
            parent, key_hash);

    return (struct ce_yng_node) {.idx = key_idx, .d = inst->doc};
}

static void create_tree_vec3(struct ce_yng_doc *_inst,
                      const char **keys,
                      uint32_t keys_count,
                      float *value) {

    struct ce_yng_node node = create_tree(_inst, keys, keys_count);
    struct ce_yng_doc *d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_seq_idx = new_node(
            d, NODE_SEQ, (struct node_value) {.node_count=3}, node.idx, key);

    struct doc_inst *inst = (struct doc_inst *) (_inst);

    ce_hash_add(&inst->key_map, key, new_seq_idx, _G.allocator);

    for (int i = 0; i < 3; ++i) {
        new_node(d, NODE_FLOAT, (struct node_value) {.f = value[2 - i]},
                 new_seq_idx, i);
    }

}


static void create_tree_bool(struct ce_yng_doc *_inst,
                      const char **keys,
                      uint32_t keys_count,
                      bool value) {
    struct ce_yng_node node = create_tree(_inst, keys, keys_count);
    struct ce_yng_doc *d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_idx;
    char *str = ce_memory_a0->str_dup(value ? "yes" : "no",
                                      _G.allocator);
    if (value) {
        new_idx = new_node(
                d,
                NODE_TRUE, (struct node_value) {.string=str},
                node.idx, key);
    } else {
        new_idx = new_node(
                d,
                NODE_FALSE, (struct node_value) {.string=str},
                node.idx, key);
    }

    struct doc_inst *inst = (struct doc_inst *) (_inst);
    ce_hash_add(&inst->key_map, key, new_idx, _G.allocator);
}

static void create_tree_float(struct ce_yng_doc *_inst,
                       const char **keys,
                       uint32_t keys_count,
                       float value) {
    struct ce_yng_node node = create_tree(_inst, keys, keys_count);
    struct ce_yng_doc *d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_idx;

    new_idx = new_node(
            d,
            NODE_FLOAT, (struct node_value) {.f = value},
            node.idx, key);

    struct doc_inst *inst = (struct doc_inst *) (_inst);
    ce_hash_add(&inst->key_map, key, new_idx, _G.allocator);
}

static void create_tree_string(struct ce_yng_doc *_inst,
                        const char **keys,
                        uint32_t keys_count,
                        const char *value) {
    struct ce_yng_node node = create_tree(_inst, keys, keys_count);
    struct ce_yng_doc *d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_idx;

    char *str = ce_memory_a0->str_dup(value, _G.allocator);

    new_idx = new_node(d,
                       NODE_STRING, (struct node_value) {.string = str},
                       node.idx, key);

    struct doc_inst *inst = (struct doc_inst *) (_inst);
    ce_hash_add(&inst->key_map, key, new_idx, _G.allocator);
}


static void destroy(struct ce_yng_doc *document) {
    struct doc_inst *inst = (struct doc_inst *) document->inst;
    struct ce_alloc *alloc = inst->alloc;

    ce_hash_free(&inst->key_map, _G.allocator);
    ce_array_free(inst->parent_file, _G.allocator);
    ce_array_free(inst->type, _G.allocator);
    ce_array_free(inst->hash, _G.allocator);
    ce_array_free(inst->value, _G.allocator);
    ce_array_free(inst->first_child, _G.allocator);
    ce_array_free(inst->next_sibling, _G.allocator);
    ce_array_free(inst->parent, _G.allocator);

    CE_FREE(alloc, inst);
    CE_FREE(alloc, document);
}

static void parent_files(struct ce_yng_doc *_inst,
                  const char ***files,
                  uint32_t *count) {
    struct doc_inst *inst = (struct doc_inst *) _inst->inst;

    *files = (const char **) inst->parent_file;
    *count = ce_array_size(inst->parent_file);
}

static struct ce_yng_doc *from_vio(struct ce_vio *vio,
                            struct ce_alloc *alloc) {

    struct ce_yng_doc *d = CE_ALLOC(alloc,
                                    struct ce_yng_doc,
                                    sizeof(struct ce_yng_doc));

    if (!d) {
        return NULL;
    }

    struct doc_inst *d_inst = CE_ALLOC(alloc,
                                       struct doc_inst,
                                       sizeof(struct doc_inst));

    if (!d_inst) {
        return NULL;
    }

    *d_inst = (struct doc_inst) {
            .alloc = alloc,
            .doc = d
    };

    *d = (struct ce_yng_doc) {
            .inst = d_inst,
            .has_key = has_key,
            .type = type,
            .hash = get_hash,
            .size = get_size,
            .get = get,
            .get_seq = get_seq,

            .as_string = as_string,
            .as_float = as_float,
            .as_bool = as_bool,
            .as_vec3 = as_vec3,
            .as_vec4 = as_vec4,
            .as_mat4 = as_mat4,

            .get_str = get_string,
            .get_float = get_float,
            .get_bool = get_bool,

            .set_float = set_float,
            .set_bool = set_bool,
            .set_str = set_string,
            .set_vec3 = set_vec3,
            .set_vec4 = set_vec4,
            .set_mat4 = set_mat4,

            .create_tree_vec3 = create_tree_vec3,
            .create_tree_bool = create_tree_bool,
            .create_tree_float = create_tree_float,
            .create_tree_string = create_tree_string,

            .foreach_dict_node = foreach_dict_node,
            .foreach_seq_node = foreach_seq_node,

            .parent_files = parent_files,
    };

    // Null node
    new_node(d, NODE_INVALID, (struct node_value) {}, 0, ~(uint64_t) (0));

    if (!parse_yaml(alloc, vio, d_inst)) {
        destroy(d);
        d = NULL;
    }

    return d;
}

static struct ce_yng_a0 yamlng_api = {
        .from_vio = from_vio,
        .save_to_vio = save_yaml,
        .destroy = destroy,

        .get_key = get_key,
        .key = calc_key,
        .combine_key = combine_key,
        .combine_key_str = combine_key_str,
};

struct ce_yng_a0 *ce_yng_a0 = &yamlng_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {.allocator = ce_memory_a0->system};

    api->register_api("ce_yng_a0", &yamlng_api);
}

static void _shutdown() {
    ce_hash_free(&_G.key_to_str, _G.allocator);
    ce_array_free(_G.key_to_str_offset, _G.allocator);
    ce_array_free(_G.key_to_str_data, _G.allocator);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        yamlng,
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