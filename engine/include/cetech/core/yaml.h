#ifndef CETECH_YAML_H
#define CETECH_YAML_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdbool.h>
#include "cetech/celib/math_types.h"

typedef struct vec3f_s vec3f_t;
typedef struct vec4f_s vec4f_t;
typedef struct mat33f_s mat33f_t;
typedef struct mat44f_s mat44f_t;

//==============================================================================
// Defines
//==============================================================================

#define yaml_is_valid(node) ((node).doc.d !=  NULL)

#define YAML_NODE_SCOPE(var_name, n, k, body)\
        do {\
            yaml_node_t var_name = yaml_get_node(n, k);\
            body \
            yaml_node_free(var_name);\
        } while(0)

#define YAML_NODE_SCOPE_IDX(var_name, n, i, body)\
        do {\
            yaml_node_t var_name = yaml_get_seq_node(n, i);\
            body \
            yaml_node_free(var_name);\
        } while(0)

//==============================================================================
// Typedefs
//==============================================================================

typedef struct yaml_document_s {
    void *d;
} yaml_document_t;

typedef struct yaml_node_s {
    yaml_document_t doc;
    uint32_t idx;
} yaml_node_t;

typedef void(*yaml_foreach_map_clb_t)(yaml_node_t key,
                                      yaml_node_t value,
                                      void *data);

typedef void(*yaml_foreach_seq_clb_t)(uint32_t idx,
                                      yaml_node_t value,
                                      void *data);

//==============================================================================
// Enums
//==============================================================================

enum yaml_node_type {
    YAML_TYPE_UNDEF,
    YAML_TYPE_NULL,
    YAML_TYPE_SCALAR,
    YAML_TYPE_SEQ,
    YAML_TYPE_MAP
};


//==============================================================================
// Interface
//==============================================================================

yaml_node_t yaml_load_str(const char *str,
                          yaml_document_t *handler);

yaml_node_t yaml_get_node(yaml_node_t node,
                          const char *key);

yaml_node_t yaml_get_seq_node(yaml_node_t seq_node,
                              size_t idx);

void yaml_node_free(yaml_node_t node);

enum yaml_node_type yaml_node_type(yaml_node_t node);

size_t yaml_node_size(yaml_node_t node);

void yaml_node_foreach_dict(yaml_node_t node,
                            yaml_foreach_map_clb_t foreach_clb,
                            void *data);

void yaml_node_foreach_seq(yaml_node_t node,
                           yaml_foreach_seq_clb_t foreach_clb,
                           void *data);

void yaml_merge(yaml_node_t root,
                yaml_node_t parent);

//==============================================================================
// Define nodes
//==============================================================================

#define YAML_NODE_AS_DEFN(T, N) T yaml_as_##N(yaml_node_t node)
#define YAML_NODE_AS_DEF(T) YAML_NODE_AS_DEFN(T, T)

YAML_NODE_AS_DEFN(int, bool);

YAML_NODE_AS_DEF(int);

YAML_NODE_AS_DEF(float);

YAML_NODE_AS_DEF(vec3f_t);

YAML_NODE_AS_DEF(vec4f_t);

YAML_NODE_AS_DEF(mat44f_t);

YAML_NODE_AS_DEF(mat33f_t);

#undef YAML_NODE_AS_DEF
#undef YAML_NODE_AS_DEFN

int yaml_as_string(yaml_node_t node,
                   char *output,
                   size_t max_len);


#ifdef __cplusplus
}
#endif

#endif //CETECH_YAML_H
