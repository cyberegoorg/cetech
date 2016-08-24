#include "celib/stringid/stringid.h"
#include "celib/crypto/hash.h"

#include "celib/string/string.h"

#define STRINGID64_SEED 22

stringid64_t stringid64_from_string(const char* str) {
    const stringid64_t id = {
            .id = hash_murmur2_64(str, str_lenght(str), STRINGID64_SEED)
    };

    return id;
}