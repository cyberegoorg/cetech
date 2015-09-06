#pragma once

#include "common/debug/asserts.h"
#include "common/string/stringid_types.h"
#include "common/crypto/murmur_hash.h"

namespace cetech {
    namespace stringid64 {
        CE_INLINE StringId64_t from_cstring_len(const char *str, const uint32_t len, const uint32_t seed=22);
        CE_INLINE StringId64_t from_cstring(const char *str, const uint32_t seed=22);
    }
    
    namespace stringid64 {
        StringId64_t from_cstring_len(const char *str, const uint32_t len, const uint32_t seed) {
            CE_CHECK_PTR(str);
            
            return murmur_hash_64(str, len, seed);
        }
        
        StringId64_t from_cstring(const char *str,const uint32_t seed) {
            CE_CHECK_PTR(str);
            
            return from_cstring_len(str, strlen(str), seed);
        }
    }
}