// based on bitsquid foundation
#pragma once

#include "types.h"

namespace cetech1 {
    /*! Implementation of the 64 bit MurmurHash2 function http://murmurhash.googlepages.com/.
     */
    uint64_t murmur_hash_64(const void* key, uint32_t len, uint64_t seed);
}