#include "celib/map.inl"

#include <cetech/api/api_system.h>
#include <cetech/config/config.h>
#include <cetech/memory/memory.h>
#include <cetech/module/module.h>
#include <cetech/yamlng/yamlng.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/log/log.h>
#include <cetech/os/vio.h>

#include <yaml.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_log_a0);

using namespace celib;

#define _G yamlng_global
#define LOG_WHERE "yamlng"

static struct _G {
} _G;

struct node_value {
    union {
        float f;
        char *string;
        uint32_t node_count;
    };
};

struct yamlng_document_inst {
    struct cel_alloc *alloc;
    struct ct_yamlng_document *doc;

    Map<uint32_t> key_map;
    Array<node_type> type;
    Array<uint64_t > hash;
    Array<node_value> value;
    Array<uint32_t> first_child;
    Array<uint32_t> next_sibling;
    Array<uint32_t> parent;
};


uint32_t new_node(ct_yamlng_document *doc,
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

void from_scalar(const uint8_t *scalar,
                 node_type *type,
                 node_value *vallue,
                 bool is_key) {
    const char *scalar_str = (const char *) scalar;
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

        } else if ((0 == strcmp(scalar_str, "n")) ||
                   (0 == strcmp(scalar_str, "no")) ||
                   (0 == strcmp(scalar_str, "false"))) {
            *type = NODE_FALSE;
        }
    }

    *type = NODE_STRING;
    *vallue = {.string = strdup(scalar_str)};
}

uint64_t hash_combine(uint64_t lhs,
                      uint64_t rhs) {
    if(lhs == 0) return rhs;
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

bool has_key(ct_yamlng_document_instance_t *_inst,
             uint64_t key) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return map::has(inst->key_map, key);
}


struct ct_yamlng_node get(ct_yamlng_document_instance_t *_inst,
                          uint64_t key) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return {.idx = map::get(inst->key_map, key, (uint32_t) 0), .d = inst->doc};
}


struct ct_yamlng_node get_seq(ct_yamlng_document_instance_t *_inst,
                                 uint64_t key,
                                 uint32_t idx) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    uint32_t n_idx = map::get(inst->key_map, key, (uint32_t) 0);
    uint32_t size = inst->value[n_idx].node_count;

    uint32_t it = inst->first_child[n_idx];
    for (int i = 1; i < size - idx; ++i) {
        it = inst->next_sibling[it];
    }

    return {.idx = it, .d = inst->doc};
}

enum node_type type(ct_yamlng_document_instance_t *_inst,
                    ct_yamlng_node node) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return inst->type[node.idx];
}

uint64_t get_hash(ct_yamlng_document_instance_t *_inst,
                  ct_yamlng_node node) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return inst->hash[node.idx];
}

uint32_t get_size(ct_yamlng_document_instance_t *_inst,
                  ct_yamlng_node node) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;
    return inst->value[node.idx].node_count;
}

const char *as_string(ct_yamlng_document_instance_t *_inst,
                      ct_yamlng_node node,
                      const char *defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    if (!node.idx) {
        return defaultt;
    }

    return inst->value[node.idx].string;
}

float as_float(ct_yamlng_document_instance_t *_inst,
               ct_yamlng_node node,
               float defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    if (!node.idx) {
        return defaultt;
    }


    return inst->value[node.idx].f;
}

bool as_bool(ct_yamlng_document_instance_t *_inst,
             ct_yamlng_node node,
             bool defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    if (!node.idx) {
        return defaultt;
    }


    return inst->type[node.idx] == NODE_TRUE;
}

void as_vec3(ct_yamlng_document_instance_t *_inst,
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

void as_vec4(ct_yamlng_document_instance_t *_inst,
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

void as_mat4(ct_yamlng_document_instance_t *_inst,
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

const char *get_string(ct_yamlng_document_instance_t *_inst,
                       uint64_t key,
                       const char *defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    ct_yamlng_node node = {.idx =map::get(inst->key_map, key, (uint32_t) 0)};
    return as_string(_inst, node, defaultt);
}

float get_float(ct_yamlng_document_instance_t *_inst,
                uint64_t key,
                float defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    ct_yamlng_node node = {.idx = map::get(inst->key_map, key, (uint32_t) 0)};
    return as_float(_inst, node, defaultt);
}

bool get_bool(ct_yamlng_document_instance_t *_inst,
              uint64_t key,
              bool defaultt) {
    yamlng_document_inst *inst = (yamlng_document_inst *) _inst;

    ct_yamlng_node node = {.idx = map::get(inst->key_map, key, (uint32_t) 0)};
    return as_bool(_inst, node, defaultt);
}


void foreach_dict_node(ct_yamlng_document_instance_t *_inst,
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

void foreach_seq_node(ct_yamlng_document_instance_t *_inst,
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
    };

    Array<parent_stack_state> parent_stack(ct_memory_a0.main_allocator());
    uint32_t parent_stack_top;
    uint32_t tmp_idx;
    uint64_t key = 0;

    array::push_back(parent_stack,
                     (parent_stack_state) {.type = NODE_INVALID, .idx = 0});

    uint8_t *source_data = CEL_ALLOCATE(alloc, uint8_t,
                                        vio->size(vio->inst) + 1);
    memset(source_data, 0, vio->size(vio->inst) + 1);
    vio->read(vio->inst, source_data, sizeof(char), vio->size(vio->inst));

    if (!yaml_parser_initialize(&parser)) {
        ct_log_a0.error(LOG_WHERE, "Failed to initialize parser");
        goto error;
    }

    yaml_parser_set_input_string(&parser, source_data,
                                 vio->size(vio->inst));

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
                from_scalar(event.data.scalar.value, &type, &value, IS_KEY());

                if (IS_KEY()) {
                    uint64_t parent_key = parent_stack[parent_stack_top].key_hash;
                    key = hash_combine(parent_key,
                                       ct_hash_a0.id64_from_str(value.string));

                    uint32_t tmp_idx = new_node(inst->doc, type, value, parent_stack[parent_stack_top].idx, key);

                    ++parent_stack[parent_stack_top].node_count;
                    array::push_back(parent_stack,
                                     (parent_stack_state) {.idx = tmp_idx, .type = NODE_STRING, .key_hash = key});

                    // VALUE_WITH_KEY
                } else if (parent_stack[parent_stack_top].type == NODE_STRING) {
                    key = parent_stack[parent_stack_top].key_hash;

                    uint32_t tmp_idx = new_node(inst->doc, type, value, parent_stack[parent_stack_top].idx, key);
                    map::set(inst->key_map, key, tmp_idx);

                    array::pop_back(parent_stack);

                    // VALUE_IN_SEQ
                } else if (parent_stack[parent_stack_top].type == NODE_SEQ) {
                    key = parent_stack[parent_stack_top].key_hash;

                    char buffer[32] = {};
                    sprintf(buffer, "%d",
                            parent_stack[parent_stack_top].node_count);

                    key = hash_combine(key, ct_hash_a0.id64_from_str(buffer));

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


static void destroy(struct ct_yamlng_document *document) {
    yamlng_document_inst *inst = (yamlng_document_inst *) document->inst;
    struct cel_alloc *alloc = inst->alloc;

    inst->key_map.destroy();
    inst->type.destroy();
    inst->hash.destroy();
    inst->value.destroy();
    inst->first_child.destroy();
    inst->next_sibling.destroy();
    inst->parent.destroy();

    CEL_FREE(alloc, inst);
    CEL_FREE(alloc, document);
}

ct_yamlng_document *from_vio(struct ct_vio *vio,
                             struct cel_alloc *alloc) {

    ct_yamlng_document *d = CEL_ALLOCATE(alloc,
                                         ct_yamlng_document,
                                         sizeof(ct_yamlng_document));
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

            .foreach_dict_node = foreach_dict_node,
            .foreach_seq_node = foreach_seq_node,
    };

    // Null node
    new_node(d, NODE_INVALID, {}, 0, ~(uint64_t)(0));

    if (!parse_yaml(alloc, vio, d_inst)) {
        goto error;
    }


    return d;

    error:
    destroy(d);
    return NULL;
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
            uint32_t size = it - begin;
            hash = hash_combine(hash,
                                ct_hash_a0.hash_murmur2_64(begin, size, 22));
            parse = false;
        }

        ++it;
    }

    uint32_t size = it - begin;
    hash = hash_combine(hash, ct_hash_a0.hash_murmur2_64(begin, size, 22));

    return hash;
}

uint64_t combine_key(uint64_t *keys,
                     uint32_t count) {
    uint64_t hash = keys[0];

    for (uint32_t i = 1; i < count; ++i) {
        hash = hash_combine(hash, keys[i]);
    }

    return hash;
}

static ct_yamlng_a0 yamlng_api = {
        .from_vio = from_vio,
        .destroy = destroy,
        .calc_key = calc_key,
        .combine_key = combine_key,
};

static void _init(ct_api_a0 *api) {
    _G = {};

    api->register_api("ct_yamlng_a0", &yamlng_api);
}

static void _shutdown() {
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
            _init(api);
        },
        {
            CEL_UNUSED(api);
            _shutdown();
        }
)