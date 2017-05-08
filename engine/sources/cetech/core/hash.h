#ifndef CETECH_HASH_H
#define CETECH_HASH_H

//==============================================================================
// Include
//==============================================================================

#include <stdint.h>

typedef struct stringid64_s {
    uint64_t id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

//! 64bit murmur 2 hash
//! \param key Key
//! \param len Key len
//! \param seed Seed
//! \return Hashed value
uint64_t hash_murmur2_64(const void *key,
                               uint64_t len,
                               uint64_t seed);

stringid64_t stringid64_from_string(const char *str);

#endif //CETECH_HASH_H
