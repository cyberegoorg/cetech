// TODO: string table

#include "celib/map.inl"

#include <cetech/api/api_system.h>
#include <cetech/config/config.h>
#include <cetech/os/memory.h>
#include <cetech/module/module.h>
#include <cetech/yaml/yamlng.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/log/log.h>
#include <cetech/os/vio.h>

#include "yaml/yaml.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_log_a0);

using namespace celib;

#define _G yamlng_global
#define LOG_WHERE "yamlng"

static struct _G {
    Map<uint32_t> key_to_str;
    Array<uint32_t> key_to_str_offset;
    Array<char> key_to_str_data;
} _G;


void add_key(const char* key, uint32_t key_len, uint64_t key_hash) {
    const uint32_t idx = array::size(_G.key_to_str_offset);
    const uint32_t offset = array::size(_G.key_to_str_data);

    array::push(_G.key_to_str_data, key, sizeof(char) * (key_len+1));
    array::push_back(_G.key_to_str_offset, offset);

    map::set(_G.key_to_str, key_hash, idx);
}

const char* get_key(uint64_t hash) {
    uint32_t  idx = map::get(_G.key_to_str, hash, UINT32_MAX);
    if(UINT32_MAX == idx) {
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

struct yamlng_document_inst {
    struct cel_alloc *alloc;
    struct ct_yng_doc *doc;

    Array<char*> parent_file;

    Map<uint32_t> key_map;
    Array<node_type> type;
    Array<uint64_t > hash;
    Array<node_value> value;
    Array<uint32_t> first_child;
    Array<uint32_t> next_sibling;
    Array<uint32_t> parent;

    bool modified;
};



uint64_t hash_combine(uint64_t lhs,
                      uint64_t rhs) {
    if(lhs == 0) return rhs;
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

uint64_t calc_key(const char *key) {
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
            const uint64_t part_hash = ct_hash_a0.hash_murmur2_64(begin, size, 22);
            add_key(begin, size, part_hash);
            hash = hash_combine(hash, part_hash);
            parse = false;
        }

        ++it;
    }

    const uint32_t size = it - begin;
    const uint64_t part_hash = ct_hash_a0.hash_murmur2_64(begin, size, 22);
    add_key(begin, size, part_hash);
    hash = hash_combine(hash, part_hash);

    return hash;
}

uint64_t combine_key(const uint64_t *keys,
                     uint32_t count) {
    uint64_t hash = keys[0];

    for (uint32_t i = 1; i < count; ++i) {
        hash = hash_combine(hash, keys[i]);
    }

    return hash;
}


uint64_t combine_key_str(const char** keys,
                            uint32_t count) {
    uint64_t hash = CT_ID64_0(keys[0]);

    for (uint32_t i = 1; i < count; ++i) {
        hash = hash_combine(hash, CT_ID64_0(keys[i]));
    }

    return hash;
}

uint32_t new_node(ct_yng_doc *doc,
                  node_type type,
                  node_value value,
                  uint32_t parent,
                  uint64_t hash) {

    yamlng_document_inst *inst = (yamlng_document_inst*)doc->inst;

    const uint32_t idx = array::size(inst->type);

    array::push_back(inst->type, type);
    array::push_back(inst->hash, hash);
    array::push_back(inst->value, value);
    array::push_back(inst->first_child, (uint32_t) 0);
    array::push_back(inst->next_sibling, (uint32_t) 0);
    array::push_back(inst->parent, parent);

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

void type_value_from_scalar(const uint8_t *scalar,
                            node_type *type,
                            node_value *vallue,
                            bool is_key) {
    const char *scalar_str = (const char *) scalar;

    if (!strlen(scalar_str)) {
        *type = NODE_STRING;
        *vallue = {.string = strdup(scalar_str)};
        return;
    }

    float f;

    if (!is_key) {
        if (sscanf(scalar_str, "%f", &f)) {
            *type = NODE_FLOAT;
            *vallue = {.f = f};
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
    *vallue = {.string = strdup(scalar_str)};
}

bool has_key(ct_yng_doc_instance_t *_inst,
             uint64_t key) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return map::has(inst->key_map, key);
}


struct ct_yamlng_node get(ct_yng_doc_instance_t *_inst,
                          uint64_t key) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return {.idx = map::get(inst->key_map, key, (uint32_t) 0), .d = inst->doc};
}


struct ct_yamlng_node get_seq(ct_yng_doc_instance_t *_inst,
                                 uint64_t key,
                                 uint32_t idx) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    uint32_t n_idx = map::get(inst->key_map, key, (uint32_t) 0);
    uint32_t size = inst->value[n_idx].node_count;

    uint32_t it = inst->first_child[n_idx];
    for (uint32_t i = 1; i < size - idx; ++i) {
        it = inst->next_sibling[it];
    }

    return {.idx = it, .d = inst->doc};
}

enum node_type type(ct_yng_doc_instance_t *_inst,
                    ct_yamlng_node node) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return inst->type[node.idx];
}

uint64_t get_hash(ct_yng_doc_instance_t *_inst,
                  ct_yamlng_node node) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return inst->hash[node.idx];
}

uint32_t get_size(ct_yng_doc_instance_t *_inst,
                  ct_yamlng_node node) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return inst->value[node.idx].node_count;
}

const char *as_string(ct_yng_doc_instance_t *_inst,
                      ct_yamlng_node node,
                      const char *defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    if (!node.idx) {
        return defaultt;
    }

    return inst->value[node.idx].string;
}

float as_float(ct_yng_doc_instance_t *_inst,
               ct_yamlng_node node,
               float defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    if (!node.idx) {
        return defaultt;
    }


    return inst->value[node.idx].f;
}

bool as_bool(ct_yng_doc_instance_t *_inst,
             ct_yamlng_node node,
             bool defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    if (!node.idx) {
        return defaultt;
    }


    return inst->type[node.idx] == NODE_TRUE;
}

void as_vec3(ct_yng_doc_instance_t *_inst,
             ct_yamlng_node node,
             float *value) {

    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

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

void as_vec4(ct_yng_doc_instance_t *_inst,
             ct_yamlng_node node,
             float *value) {

    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

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

void as_mat4(ct_yng_doc_instance_t *_inst,
             ct_yamlng_node node,
             float *value) {

    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

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

const char *get_string(ct_yng_doc_instance_t *_inst,
                       uint64_t key,
                       const char *defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    ct_yamlng_node node = {.idx =map::get(inst->key_map, key, (uint32_t) 0)};
    return as_string(_inst, node, defaultt);
}

float get_float(ct_yng_doc_instance_t *_inst,
                uint64_t key,
                float defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    ct_yamlng_node node = {.idx = map::get(inst->key_map, key, (uint32_t) 0)};
    return as_float(_inst, node, defaultt);
}

bool get_bool(ct_yng_doc_instance_t *_inst,
              uint64_t key,
              bool defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    ct_yamlng_node node = {.idx = map::get(inst->key_map, key, (uint32_t) 0)};
    return as_bool(_inst, node, defaultt);
}

void set_float(ct_yng_doc_instance_t *_inst, struct ct_yamlng_node node, float value) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    inst->value[node.idx].f = value;
    inst->modified = true;
}

void set_bool(ct_yng_doc_instance_t *_inst,
                 struct ct_yamlng_node node,
                 bool value){
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    inst->type[node.idx] = value ? NODE_TRUE : NODE_FALSE;
    inst->modified = true;
}


void set_string(ct_yng_doc_instance_t *_inst,
                   ct_yamlng_node node,
                   const char *value) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    cel_alloc* alloc = ct_memory_a0.main_allocator();
    char *str = inst->value[node.idx].string;

    CEL_FREE(alloc, str);

    str = ct_memory_a0.str_dup(value, alloc);
    inst->value[node.idx].string = str;
    inst->modified = true;
}

void set_vec3(ct_yng_doc_instance_t *_inst,
                 ct_yamlng_node node,
                 float *value) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    uint32_t it = inst->first_child[node.idx];
    inst->value[it].f = value[2];

    it = inst->next_sibling[it];
    inst->value[it].f = value[1];

    it = inst->next_sibling[it];
    inst->value[it].f = value[0];

    inst->modified = true;
}

void set_vec4(ct_yng_doc_instance_t *_inst,
                 ct_yamlng_node node,
                 float *value) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

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

void set_mat4(ct_yng_doc_instance_t *_inst,
                 ct_yamlng_node node,
                 float *value) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

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

void foreach_dict_node(ct_yng_doc_instance_t *_inst,
                       struct ct_yamlng_node node,
                       ct_yamlng_foreach_map_t foreach_clb,
                       void *data) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    uint32_t it = inst->first_child[node.idx];
    while (0 != it) {
        ct_yamlng_node key = {.idx=it, .d = inst->doc};
        ct_yamlng_node value = {.idx=inst->first_child[it], .d = inst->doc};

        foreach_clb(key, value, data);

        it = inst->next_sibling[it];
    }
}

void foreach_seq_node(ct_yng_doc_instance_t *_inst,
                      struct ct_yamlng_node node,
                      ct_yamlng_foreach_seq_t foreach_clb,
                      void *data) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    uint32_t idx = 0;
    uint32_t it = inst->first_child[node.idx];
    while (0 != it) {
        ct_yamlng_node value = {.idx=it, .d = inst->doc};

        foreach_clb(idx, value, data);

        it = inst->next_sibling[it];
        ++idx;
    }
}


bool parse_yaml(struct cel_alloc *alloc,
                struct ct_vio *vio,
                struct yamlng_document_inst *inst) {
    yaml_parser_t parser;
    yaml_event_t event;

    struct parent_stack_state {
        node_type type;
        uint32_t idx;
        uint32_t node_count;
        uint64_t key_hash;
        uint64_t str_hash;
    };

    Array<parent_stack_state> parent_stack(ct_memory_a0.main_allocator());
    uint32_t parent_stack_top;
    uint32_t tmp_idx;
    uint64_t key = 0;

    array::push_back(parent_stack,
                     (parent_stack_state) {.type = NODE_INVALID, .idx = 0});

    uint8_t *source_data = CEL_ALLOCATE(alloc, uint8_t,
                                        vio->size(vio) + 1);
    memset(source_data, 0, vio->size(vio) + 1);
    vio->read(vio, source_data, sizeof(char), vio->size(vio));

    if (!yaml_parser_initialize(&parser)) {
        ct_log_a0.error(LOG_WHERE, "Failed to initialize parser");
        goto error;
    }

    yaml_parser_set_input_string(&parser, source_data,
                                 vio->size(vio));

#define IS_KEY() (parent_stack[parent_stack_top].type == NODE_MAP)
#define HAS_KEY() (parent_stack[parent_stack_top].type == NODE_STRING)
    do {
        parent_stack_top = array::size(parent_stack) - 1;

        if (!yaml_parser_parse(&parser, &event)) {
            ct_log_a0.error(LOG_WHERE, "Parser error %d\n", parser.error);
            goto error;
        }

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

                tmp_idx = new_node(inst->doc, NODE_SEQ, {},
                                   parent_stack[parent_stack_top].idx, key);

                map::set(inst->key_map, key, tmp_idx);

                if (HAS_KEY()) {
                    array::pop_back(parent_stack);
                }

                ++parent_stack[parent_stack_top].node_count;
                array::push_back(parent_stack,
                                 (parent_stack_state) {.idx = tmp_idx, .type = NODE_SEQ, .key_hash =key});
                break;

            case YAML_SEQUENCE_END_EVENT:
                inst->value[parent_stack[parent_stack_top].idx].node_count = parent_stack[parent_stack_top].node_count;
                array::pop_back(parent_stack);
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

                tmp_idx = new_node(inst->doc, NODE_MAP, {},
                                   parent_stack[parent_stack_top].idx, key);

                map::set(inst->key_map, key, tmp_idx);

                if (HAS_KEY()) {
                    array::pop_back(parent_stack);
                }

                ++parent_stack[parent_stack_top].node_count;
                array::push_back(parent_stack,
                                 (parent_stack_state) {.idx = tmp_idx, .type = NODE_MAP, .key_hash = key});
                break;

            case YAML_MAPPING_END_EVENT:
                inst->value[parent_stack[parent_stack_top].idx].node_count = parent_stack[parent_stack_top].node_count;
                array::pop_back(parent_stack);
                break;

            case YAML_ALIAS_EVENT:
                break;

            case YAML_SCALAR_EVENT: {
                node_type type;
                node_value value;
                type_value_from_scalar(event.data.scalar.value,
                                       &type, &value, IS_KEY());

                static const uint64_t PARENT_KEY = CT_ID64_0("PARENT");

                if (IS_KEY()) {
                    uint64_t key_hash = CT_ID64_0(value.string);
                    uint64_t parent_key = parent_stack[parent_stack_top].key_hash;
                    key = hash_combine(parent_key, key_hash);

                    uint32_t tmp_idx = new_node(inst->doc, type, value, parent_stack[parent_stack_top].idx, key);

                    ++parent_stack[parent_stack_top].node_count;
                    array::push_back(parent_stack,
                                     (parent_stack_state) {.idx = tmp_idx, .type = NODE_STRING, .key_hash = key, .str_hash = key_hash});

                // VALUE_WITH_KEY
                } else if (parent_stack[parent_stack_top].type == NODE_STRING) {
                    key = parent_stack[parent_stack_top].key_hash;

                    if(PARENT_KEY == parent_stack[parent_stack_top].str_hash) {
                        array::push_back(inst->parent_file, value.string);
                    }

                    uint32_t tmp_idx = new_node(inst->doc, type, value, parent_stack[parent_stack_top].idx, key);
                    map::set(inst->key_map, key, tmp_idx);

                    array::pop_back(parent_stack);

                // VALUE_IN_SEQ
                } else if (parent_stack[parent_stack_top].type == NODE_SEQ) {
                    key = parent_stack[parent_stack_top].key_hash;

                    char buffer[32] = {};
                    sprintf(buffer, "%d",
                            parent_stack[parent_stack_top].node_count);

                    key = hash_combine(key, calc_key(buffer));

                    uint32_t tmp_idx = new_node(inst->doc, type, value, parent_stack[parent_stack_top].idx, key);
                    map::set(inst->key_map, key, tmp_idx);

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

void save_recursive(yamlng_document_inst* inst, uint32_t root, yaml_emitter_t *emitter) {
    node_type type = inst->type[root];
    node_value value = inst->value[root];
    char buffer[128];

    yaml_event_t event;

    switch (type) {
        case NODE_FLOAT:
            sprintf(buffer, "%f", value.f);
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *)buffer, strlen(buffer),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_STRING:
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *)value.string, strlen(value.string),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);
            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_TRUE:
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *)"yes", strlen("yes"),
                                         1, 1, YAML_PLAIN_SCALAR_STYLE);

            if (!yaml_emitter_emit(emitter, &event))
                goto error;

            break;

        case NODE_FALSE:
            yaml_scalar_event_initialize(&event,
                                         NULL,
                                         NULL,
                                         (unsigned char *)"no", strlen("no"),
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

        case NODE_SEQ:{
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

int write_handler(void *ext, unsigned char *buffer, size_t size) {
    ct_vio *output = (ct_vio *)ext;
    output->write(output, buffer, sizeof(unsigned char), size);
    return 1;
}

bool save_yaml(struct cel_alloc *alloc,
               struct ct_vio *vio,
               struct ct_yng_doc *doc) {
//    unsigned char kim_nuke_shit[4096];

    yaml_emitter_t emitter;
    yaml_event_t event;

    ct_yng_doc* d  = doc;
    yamlng_document_inst* inst = (yamlng_document_inst *)(d->inst);
    ct_yamlng_node root_node = d->get(d->inst, 0);


    if(!yaml_emitter_initialize(&emitter)) {
        ct_log_a0.error(LOG_WHERE, "Could not initialize YAML emiter.");
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
    ct_log_a0.error(LOG_WHERE, "Error emmit");
    yaml_emitter_delete(&emitter);

    return false;
}

ct_yamlng_node create_tree(ct_yng_doc_instance_t *_inst,
                              const char **keys,
                              uint32_t keys_count) {
    uint32_t first_nonexist = 0;
    uint64_t last_exist_key = 0;
    for (int i = 0; i < keys_count; ++i) {
        uint64_t key = combine_key_str(keys, i+1);
        if(!has_key(_inst, key)) {
            first_nonexist = i;
            break;
        } else {
            last_exist_key = key;
        }
    }

    ct_yamlng_node n = get(_inst, last_exist_key);
    yamlng_document_inst* inst = (yamlng_document_inst *)(_inst);

    uint32_t parent = n.idx;
    for (int i = first_nonexist; i < keys_count-1; ++i) {
        uint64_t key = combine_key_str(keys, i+1);
        inst->value[parent].node_count += 1;

        uint32_t key_idx = new_node(
                inst->doc,
                NODE_STRING,
                {.string = ct_memory_a0.str_dup(keys[i], ct_memory_a0.main_allocator())},
                parent, key);

        uint32_t new_map_idx = new_node(
                inst->doc,
                NODE_MAP, {.node_count=0},
                key_idx, key);

        map::set(inst->key_map, key, new_map_idx);

        parent = new_map_idx;
    }

    uint64_t key_hash = CT_ID64_0(keys[keys_count-1]);
    inst->value[parent].node_count += 1;
    uint32_t key_idx = new_node(
            inst->doc,
            NODE_STRING,
            {.string = ct_memory_a0.str_dup(keys[keys_count-1], ct_memory_a0.main_allocator())},
            parent, key_hash);

    return {.idx = key_idx, .d = inst->doc};
}

void create_tree_vec3(ct_yng_doc_instance_t *_inst,
                      const char **keys,
                      uint32_t keys_count,
                      float *value){

    ct_yamlng_node node = create_tree(_inst, keys, keys_count);
    ct_yng_doc* d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_seq_idx = new_node(
            d,
            NODE_SEQ, {.node_count=3},
            node.idx, key);

    yamlng_document_inst* inst = (yamlng_document_inst *)(_inst);

    map::set(inst->key_map, key, new_seq_idx);

    for (int i = 0; i < 3; ++i) {
        new_node(
                d,
                NODE_FLOAT, {.f = value[2-i]},
                new_seq_idx, i);
    }

}


void create_tree_bool(ct_yng_doc_instance_t *_inst,
                         const char **keys,
                         uint32_t keys_count,
                         bool value) {
    ct_yamlng_node node = create_tree(_inst, keys, keys_count);
    ct_yng_doc* d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_idx;
    char* str =  ct_memory_a0.str_dup(value ? "yes" : "no", ct_memory_a0.main_allocator());
    if(value) {
        new_idx = new_node(
                d,
                NODE_TRUE, {.string=str},
                node.idx, key);
    } else {
        new_idx = new_node(
                d,
                NODE_FALSE, {.string=str},
                node.idx, key);
    }

    yamlng_document_inst* inst = (yamlng_document_inst *)(_inst);
    map::set(inst->key_map, key, new_idx);
}

void create_tree_float(ct_yng_doc_instance_t *_inst,
                          const char **keys,
                          uint32_t keys_count,
                          float value) {
    ct_yamlng_node node = create_tree(_inst, keys, keys_count);
    ct_yng_doc* d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_idx;

    new_idx = new_node(
            d,
            NODE_FLOAT, {.f = value},
            node.idx, key);

    yamlng_document_inst* inst = (yamlng_document_inst *)(_inst);
    map::set(inst->key_map, key, new_idx);
}

void create_tree_string(ct_yng_doc_instance_t *_inst,
                           const char **keys,
                           uint32_t keys_count,
                           const char* value) {
    ct_yamlng_node node = create_tree(_inst, keys, keys_count);
    ct_yng_doc* d = node.d;

    uint64_t key = combine_key_str(keys, keys_count);

    uint32_t new_idx;

    char* str = ct_memory_a0.str_dup(value, ct_memory_a0.main_allocator());

    new_idx = new_node(
            d,
            NODE_STRING, {.string = str},
            node.idx, key);

    yamlng_document_inst* inst = (yamlng_document_inst *)(_inst);
    map::set(inst->key_map, key, new_idx);
}


static void destroy(struct ct_yng_doc *document) {
    yamlng_document_inst *inst = (yamlng_document_inst *) document->inst;
    struct cel_alloc *alloc = inst->alloc;

    inst->key_map.destroy();
    inst->parent_file.destroy();
    inst->type.destroy();
    inst->hash.destroy();
    inst->value.destroy();
    inst->first_child.destroy();
    inst->next_sibling.destroy();
    inst->parent.destroy();

    CEL_FREE(alloc, inst);
    CEL_FREE(alloc, document);
}

void parent_files(ct_yng_doc_instance_t *_inst, const char*** files, uint32_t *count){
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    *files = (const char **)array::begin(inst->parent_file);
    *count = array::size(inst->parent_file);
}

ct_yng_doc *from_vio(struct ct_vio *vio,
                             struct cel_alloc *alloc) {

    ct_yng_doc *d = CEL_ALLOCATE(alloc,
                                         ct_yng_doc,
                                         sizeof(ct_yng_doc));
    if (!d) {
        return NULL;
    }

    yamlng_document_inst *d_inst = CEL_ALLOCATE(alloc,
                                                yamlng_document_inst,
                                                sizeof(yamlng_document_inst));
    if (!d_inst) {
        return NULL;
    }

    *d_inst = {
            .alloc = alloc,
            .doc = d
    };

    d_inst->key_map.init(alloc);
    d_inst->parent_file.init(alloc);
    d_inst->type.init(alloc);
    d_inst->hash.init(alloc);
    d_inst->value.init(alloc);
    d_inst->first_child.init(alloc);
    d_inst->next_sibling.init(alloc);
    d_inst->parent.init(alloc);

    *d = {
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

            .get_string = get_string,
            .get_float = get_float,
            .get_bool = get_bool,

            .set_float = set_float,
            .set_bool = set_bool,
            .set_string = set_string,
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
    new_node(d, NODE_INVALID, {}, 0, ~(uint64_t)(0));

    if (!parse_yaml(alloc, vio, d_inst)) {
        destroy(d);
        d = NULL;
    }

    return d;
}

static ct_yng_a0 yamlng_api = {
        .from_vio = from_vio,
        .save_to_vio = save_yaml,
        .destroy = destroy,

        .get_key = get_key,
        .calc_key = calc_key,
        .combine_key = combine_key,
        .combine_key_str = combine_key_str,
};

static void _init(ct_api_a0 *api) {
    _G = {};

    _G.key_to_str.init(ct_memory_a0.main_allocator());
    _G.key_to_str_offset.init(ct_memory_a0.main_allocator());
    _G.key_to_str_data.init(ct_memory_a0.main_allocator());

    api->register_api("ct_yng_a0", &yamlng_api);
}

static void _shutdown() {
    _G.key_to_str.destroy();
    _G.key_to_str_offset.destroy();
    _G.key_to_str_data.destroy();

    _G = {};
}

CETECH_MODULE_DEF(
        yamlng,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)