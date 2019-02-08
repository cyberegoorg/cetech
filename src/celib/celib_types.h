#ifndef CETECH_CELIB_TYPES_H
#define CETECH_CELIB_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>


#ifdef DEBUG

#define CE_ASSERT(where, condition)                                         \
    do {                                                                    \
        if (!(condition)) {                                                 \
            ce_os_error_a0->assert(where, #condition, __FILE__, __LINE__); \
        }                                                                   \
    } while (0)
#else
#define CE_ASSERT(where, condition) \
    do {} while (0)
#endif


typedef struct ce_vec2_t {
    float x, y;
} ce_vec2_t;

typedef struct ce_vec3_t {
    float x, y, z;
} ce_vec3_t;

typedef struct ce_vec4_t {
    float x, y, z, w;
} ce_vec4_t;

typedef struct ce_time_t {
    uint64_t o;
} ce_time_t;

#define CE_ID64_0(str, hash) hash

#if defined(CE_DYNAMIC_MODULE)
#define CE_MODULE_API static
#else
#define CE_MODULE_API extern
#endif

#define CE_MODULE(name) CE_MODULE_API struct name* name

#include <celib/os/error.h>

#ifdef __cplusplus
}
#endif

#endif //CETECH_CELIB_TYPES_H
