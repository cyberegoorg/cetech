#ifndef CELIB_STRINGID_H
#define CELIB_STRINGID_H

//==============================================================================
// Includes
//==============================================================================

#include "string.h"
#include "../types.h"
#include "../crypto/hash.h"

#define STRINGID64_SEED 22

typedef struct {
    u64 id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

static stringid64_t stringid64_from_string(const char *str) {
    const stringid64_t id = {
            .id = celib_hash_murmur2_64(str, str_lenght(str), STRINGID64_SEED)
    };

    return id;
}

#endif //CELIB_STRINGID_H
