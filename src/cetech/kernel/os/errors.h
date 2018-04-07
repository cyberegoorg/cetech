#ifndef CETECH_ERRORS_H
#define CETECH_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <stdio.h>

//==============================================================================
// Defines
//==============================================================================

#ifdef CETECH_DEBUG
#define CETECH_ASSERT(where, condition)                             \
    do {                                                            \
        if (!(condition)) {                                         \
            ct_error_assert(where, #condition, __FILE__, __LINE__); \
        }                                                           \
    } while (0)
#else
#define CETECH_ASSERT(where, condition) do {} while (0)
#endif


//==============================================================================
// Iterface
//==============================================================================

struct ct_error_a0 {
    void (*assert)(const char *where,
                   const char *condition,
                   const char *filename,
                   int line);
};

void ct_error_assert(const char *where,
                     const char *condition,
                     const char *filename,
                     int line);

#ifdef __cplusplus
}
#endif

#endif //CETECH_ERRORS_H
