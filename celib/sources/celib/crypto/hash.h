#ifndef CELIB_HASH_H
#define CELIB_HASH_H

//==============================================================================
// Include
//==============================================================================

#include "../types.h"

//==============================================================================
// Interface
//==============================================================================

//! 64bit murmur 2 hash
//! \param key Key
//! \param len Key len
//! \param seed Seed
//! \return Hashed value
static u64 celib_hash_murmur2_64(const void *key,
                                 u64 len,
                                 uint64_t seed) {

    static const u64 m = 0xc6a4a7935bd1e995ULL;
    static const u32 r = 47;

    u64 h = seed ^(len * m);

    const u64 *data = (const u64 *) key;
    const u64 *end = data + (len / 8);

    while (data != end) {

#ifdef CELIB_BIG_ENDIAN
        u64 k = *data++;
        char* p = (char*)&k;
        char c;
        c = p[0]; p[0] = p[7]; p[7] = c;
        c = p[1]; p[1] = p[6]; p[6] = c;
        c = p[2]; p[2] = p[5]; p[5] = c;
        c = p[3]; p[3] = p[4]; p[4] = c;
#else
        u64 k = *data++;
#endif

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = (const unsigned char *) data;

    switch (len & 7) {
        case 7:
            h ^= ((u64) data2[6]) << 48;
        case 6:
            h ^= ((u64) data2[5]) << 40;
        case 5:
            h ^= ((u64) data2[4]) << 32;
        case 4:
            h ^= ((u64) data2[3]) << 24;
        case 3:
            h ^= ((u64) data2[2]) << 16;
        case 2:
            h ^= ((u64) data2[1]) << 8;
        case 1:
            h ^= ((u64) data2[0]);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

#endif //CELIB_HASH_H
