
#include "celib/string.h"
#include "celib/hash.h"

#define STRINGID64_SEED 22

typedef struct {
    u64 id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

stringid64_t stringid64_from_string(const char *str) {
    const stringid64_t id = {
            .id = cel_hash_murmur2_64(str, cel_strlen(str), STRINGID64_SEED)
    };

    return id;
}
