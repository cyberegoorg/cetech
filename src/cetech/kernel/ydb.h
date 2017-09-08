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
struct ct_yng_doc;

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
    ct_yng_doc *(*get)(const char *path);

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

    void (*set_float)(const char *path,
                      uint64_t* keys,
                      uint32_t keys_count,
                      float value);

    void (*set_bool)(const char *path,
                     uint64_t* keys,
                     uint32_t keys_count,
                     bool value);


    void (*set_string)(const char *path,
                       uint64_t* keys,
                       uint32_t keys_count,
                       const char *value);

    void (*set_vec3)(const char *path,
                     uint64_t* keys,
                     uint32_t keys_count,
                     float *value);

    void (*set_vec4)(const char *path,
                     uint64_t* keys,
                     uint32_t keys_count,
                     float *value);

    void (*set_mat4)(const char *path,
                     uint64_t* keys,
                     uint32_t keys_count,
                     float *value);

    void (*parent_files)(const char* path,
                         const char ***files,
                         uint32_t *count);

    void (*save)(const char* path);
    void (*save_all_modified)();

    void (*check_fs)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_YDB_H
