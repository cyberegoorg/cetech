#ifndef CETECH_YDB_H
#define CETECH_YDB_H

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
struct ct_yamlng_document;

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

struct ct_ydb_a0 {
    ct_yamlng_document *(*get)(const char *path);

    void (*free)(const char *path);


    bool (*has_key)(const char *path,
                    uint64_t *keys,
                    uint32_t keys_count);

    const char *(*get_string)(const char *path,
                              uint64_t *keys,
                              uint32_t keys_count,
                              const char *defaultt);

    float (*get_float)(const char *path,
                       uint64_t *keys,
                       uint32_t keys_count,
                       float defaultt);

    bool (*get_bool)(const char *path,
                     uint64_t *keys,
                     uint32_t keys_count,
                     bool defaultt);

    void (*get_vec3)(const char *path,
                     uint64_t *keys,
                     uint32_t keys_count,
                     float v[3],
                     float defaultt[3]);

    void (*get_vec4)(const char *path,
                     uint64_t *keys,
                     uint32_t keys_count,
                     float v[4],
                     float defaultt[4]);


    void (*get_mat4)(const char *path,
                     uint64_t *keys,
                     uint32_t keys_count,
                     float v[16],
                     float defaultt[16]);

    void (*get_map_keys)(const char *path,
                         uint64_t *keys,
                         uint32_t keys_count,
                         uint64_t *map_keys,
                         uint32_t max_map_keys,
                         uint32_t *map_keys_count);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_YDB_H
