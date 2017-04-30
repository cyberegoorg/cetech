#include <stddef.h>

#include <cetech/allocator.h>
#include "cetech/string.h"
#include "cetech/hash.h"

#define STRINGID64_SEED 22

typedef struct {
    uint64_t id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

stringid64_t stringid64_from_string(const char *str) {
    const stringid64_t id = {
            .id = hash_murmur2_64(str, strlen(str), STRINGID64_SEED)
    };

    return id;
}
