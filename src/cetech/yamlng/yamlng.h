#ifndef CETECH_YAMLNG_H
#define CETECH_YAMLNG_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdbool.h>

struct ct_vio;
struct cel_alloc;

//==============================================================================
// Defines
//==============================================================================

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Enums
//==============================================================================


//==============================================================================
// Interface
//==============================================================================

typedef void ct_yamlng_document_instance_t;

enum node_type {
    NODE_INVALID = 0,
    NODE_FLOAT,
    NODE_STRING,
    NODE_TRUE,
    NODE_FALSE,
    NODE_MAP,
    NODE_SEQ,
};


struct ct_yamlng_document;

struct ct_yamlng_node {
    struct ct_yamlng_document *d;
    uint32_t idx;
};

typedef void(*ct_yamlng_foreach_map_t)(
        struct ct_yamlng_node key,
        struct ct_yamlng_node value,
        void *data);

typedef void(*ct_yamlng_foreach_seq_t)(
        uint32_t idx,
        struct ct_yamlng_node value,
        void *data);

struct ct_yamlng_document {
    ct_yamlng_document_instance_t *inst;

    bool (*has_key)(ct_yamlng_document_instance_t *inst,
                    uint64_t key);


    struct ct_yamlng_node (*get)(ct_yamlng_document_instance_t *inst,
                                 uint64_t key);

    struct ct_yamlng_node (*get_seq)(ct_yamlng_document_instance_t *inst,
                                     uint64_t key,
                                     uint32_t idx);

    enum node_type (*type)(ct_yamlng_document_instance_t *inst,
                           ct_yamlng_node node);

    uint64_t (*hash)(ct_yamlng_document_instance_t *inst,
                     ct_yamlng_node node);

    uint32_t (*size)(ct_yamlng_document_instance_t *inst,
                     ct_yamlng_node node);

    const char *(*as_string)(ct_yamlng_document_instance_t *inst,
                             ct_yamlng_node node,
                             const char *defaultt);

    float (*as_float)(ct_yamlng_document_instance_t *inst,
                      ct_yamlng_node node,
                      float defaultt);

    bool (*as_bool)(ct_yamlng_document_instance_t *inst,
                    ct_yamlng_node node,
                    bool defaultt);

    void (*as_vec3)(ct_yamlng_document_instance_t *inst,
                    ct_yamlng_node node,
                    float *value);

    void (*as_vec4)(ct_yamlng_document_instance_t *inst,
                    ct_yamlng_node node,
                    float *value);

    void (*as_mat4)(ct_yamlng_document_instance_t *inst,
                    ct_yamlng_node node,
                    float *value);

    const char *(*get_string)(ct_yamlng_document_instance_t *inst,
                              uint64_t key,
                              const char *defaultt);

    float (*get_float)(ct_yamlng_document_instance_t *inst,
                       uint64_t key,
                       float defaultt);

    bool (*get_bool)(ct_yamlng_document_instance_t *inst,
                     uint64_t key,
                     bool defaultt);

    void (*foreach_dict_node)(ct_yamlng_document_instance_t *inst,
                              struct ct_yamlng_node node,
                              ct_yamlng_foreach_map_t foreach_clb,
                              void *data);

    void (*foreach_seq_node)(ct_yamlng_document_instance_t *inst,
                             struct ct_yamlng_node node,
                             ct_yamlng_foreach_seq_t foreach_clb,
                             void *data);
};


struct ct_yamlng_a0 {
    ct_yamlng_document *(*from_vio)(struct ct_vio *vio,
                                    struct cel_alloc *alloc);

    void (*destroy)(struct ct_yamlng_document *document);

    uint64_t (*calc_key)(const char *key);

    uint64_t (*combine_key)(uint64_t *keys,
                            uint32_t count);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_YAMLNG_H
