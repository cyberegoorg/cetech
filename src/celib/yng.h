#ifndef CE_YAMLNG_H
#define CE_YAMLNG_H

#include <stddef.h>
#include <stdint.h>



//==============================================================================
// Includes
//==============================================================================

#include <stdbool.h>
#include <celib/module.inl>

struct ce_vio;
struct ce_alloc;

typedef void ce_yng_doc_instance_t;

enum node_type {
    NODE_INVALID = 0,
    NODE_FLOAT = 1,
    NODE_STRING = 2,
    NODE_TRUE = 3,
    NODE_FALSE = 4,
    NODE_MAP = 5,
    NODE_SEQ = 6,
};


struct ce_yng_doc;

struct ce_yng_node {
    struct ce_yng_doc *d;
    uint32_t idx;
};

typedef void(*ce_yng_foreach_map_t)(
        struct ce_yng_node key,
        struct ce_yng_node value,
        void *data);

typedef void(*ce_yng_foreach_seq_t)(
        uint32_t idx,
        struct ce_yng_node value,
        void *data);

struct ce_yng_doc {
    ce_yng_doc_instance_t *inst;

    bool (*has_key)(struct ce_yng_doc *doc,
                    uint64_t key);

    struct ce_yng_node (*get)(struct ce_yng_doc *doc,
                                 uint64_t key);

    struct ce_yng_node (*get_seq)(struct ce_yng_doc *doc,
                                     uint64_t key,
                                     uint32_t idx);

    enum node_type (*type)(struct ce_yng_doc *doc,
                           struct ce_yng_node node);

    uint64_t (*hash)(struct ce_yng_doc *doc,
                     struct ce_yng_node node);

    uint32_t (*size)(struct ce_yng_doc *doc,
                     struct ce_yng_node node);

    const char *(*as_string)(struct ce_yng_doc *doc,
                             struct ce_yng_node node,
                             const char *defaultt);

    float (*as_float)(struct ce_yng_doc *doc,
                      struct ce_yng_node node,
                      float defaultt);

    bool (*as_bool)(struct ce_yng_doc *doc,
                    struct ce_yng_node node,
                    bool defaultt);

    void (*as_vec3)(struct ce_yng_doc *doc,
                    struct ce_yng_node node,
                    float *value);

    void (*as_vec4)(struct ce_yng_doc *doc,
                    struct ce_yng_node node,
                    float *value);

    void (*as_mat4)(struct ce_yng_doc *doc,
                    struct ce_yng_node node,
                    float *value);

    const char *(*get_str)(struct ce_yng_doc *doc,
                           uint64_t key,
                           const char *defaultt);

    float (*get_float)(struct ce_yng_doc *doc,
                       uint64_t key,
                       float defaultt);

    bool (*get_bool)(struct ce_yng_doc *doc,
                     uint64_t key,
                     bool defaultt);

    void (*set_float)(struct ce_yng_doc *doc,
                      struct ce_yng_node node,
                      float value);

    void (*set_bool)(struct ce_yng_doc *doc,
                     struct ce_yng_node node,
                     bool value);


    void (*set_str)(struct ce_yng_doc *doc,
                    struct ce_yng_node node,
                    const char *value);

    void (*set_vec3)(struct ce_yng_doc *doc,
                     struct ce_yng_node node,
                     float *value);

    void (*set_vec4)(struct ce_yng_doc *doc,
                     struct ce_yng_node node,
                     float *value);

    void (*set_mat4)(struct ce_yng_doc *doc,
                     struct ce_yng_node node,
                     float *value);

    void (*create_tree_vec3)(struct ce_yng_doc *doc,
                             const char **keys,
                             uint32_t keys_count,
                             float *value);

    void (*create_tree_bool)(struct ce_yng_doc *doc,
                             const char **keys,
                             uint32_t keys_count,
                             bool value);

    void (*create_tree_float)(struct ce_yng_doc *doc,
                              const char **keys,
                              uint32_t keys_count,
                              float value);

    void (*create_tree_string)(struct ce_yng_doc *doc,
                               const char **keys,
                               uint32_t keys_count,
                               const char *value);

    void (*foreach_dict_node)(struct ce_yng_doc *doc,
                              struct ce_yng_node node,
                              ce_yng_foreach_map_t foreach_clb,
                              void *data);

    void (*foreach_seq_node)(struct ce_yng_doc *doc,
                             struct ce_yng_node node,
                             ce_yng_foreach_seq_t foreach_clb,
                             void *data);

    void (*parent_files)(struct ce_yng_doc *doc,
                         const char ***files,
                         uint32_t *count);
};


struct ce_yng_a0 {
    struct ce_yng_doc *(*from_vio)(struct ce_vio *vio,
                                   struct ce_alloc *alloc);

    bool (*save_to_vio)(struct ce_alloc *alloc,
                        struct ce_vio *vio,
                        struct ce_yng_doc *doc);

    void (*destroy)(struct ce_yng_doc *document);

    const char *(*get_key)(uint64_t hash);

    uint64_t (*key)(const char *key);

    uint64_t (*combine_key)(const uint64_t *keys,
                            uint32_t count);

    uint64_t (*combine_key_str)(const char **keys,
                                uint32_t count);
};

CE_MODULE(ce_yng_a0);

#endif //CE_YAMLNG_H
