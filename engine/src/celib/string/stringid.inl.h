#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "celib/asserts.h"
#include "celib/stringid_types.h"
#include "celib/crypto/murmur_hash.inl.h"

namespace cetech {

    /***************************************************************************
    **** StringId64 interface
    ***************************************************************************/
    namespace stringid64 {

        /***********************************************************************
        **** Create StringId64 from string and string len
        ***********************************************************************/
        CE_INLINE StringId64_t from_cstringn(const char* str,
                                             const uint32_t len,
                                             const uint32_t seed = 22);

        /***********************************************************************
        **** Create StringId64 from zero terminated string
        ***********************************************************************/
        CE_INLINE StringId64_t from_cstring(const char* str,
                                            const uint32_t seed = 22);
    }

    /***************************************************************************
    **** StringId64 implementation
    ***************************************************************************/
    namespace stringid64 {
        StringId64_t from_cstringn(const char* str,
                                   const uint32_t len,
                                   const uint32_t seed) {
            CE_ASSERT("stringid64", str != nullptr);

            return murmur_hash_64(str, len, seed);
        }

        StringId64_t from_cstring(const char* str,
                                  const uint32_t seed) {
            CE_ASSERT("stringid64", str != nullptr);

            return from_cstringn(str, strlen(str), seed);
        }
    }
}
