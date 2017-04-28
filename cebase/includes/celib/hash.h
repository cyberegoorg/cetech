#ifndef CELIB_HASH_H
#define CELIB_HASH_H

//==============================================================================
// Include
//==============================================================================

#include <stdint.h>

//==============================================================================
// Interface
//==============================================================================

//! 64bit murmur 2 hash
//! \param key Key
//! \param len Key len
//! \param seed Seed
//! \return Hashed value
uint64_t cel_hash_murmur2_64(const void *key,
                               uint64_t len,
                               uint64_t seed);
#endif //CELIB_HASH_H
