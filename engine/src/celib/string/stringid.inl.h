#pragma once

#include "celib/asserts.h"
#include "celib/string/stringid_types.h"
#include "celib/crypto/murmur_hash.inl.h"

namespace cetech {
    namespace stringid64 {
        CE_INLINE StringId64_t from_cstringn(const char* str,
                                             const uint32_t len,
                                             const uint32_t seed = 22);
        CE_INLINE StringId64_t from_cstring(const char* str,
                                            const uint32_t seed = 22);
    }

    namespace stringid64 {
        StringId64_t from_cstringn(const char* str,
                                   const uint32_t len,
                                   const uint32_t seed) {
            CE_CHECK_PTR(str);

            return murmur_hash_64(str, len, seed);
        }

        StringId64_t from_cstring(const char* str,
                                  const uint32_t seed) {
            CE_CHECK_PTR(str);

            return from_cstringn(str, strlen(str), seed);
        }
    }
}
