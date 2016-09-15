#ifndef CETECH_HASH_H
#define CETECH_HASH_H

//==============================================================================
// Include
//==============================================================================

#include <stdint.h>
#include <stddef.h>


//==============================================================================
// Interface
//==============================================================================


//! 64bit murmur 2 hash
//! \param key Key
//! \param len Key len
//! \param seed Seed
//! \return Hashed value
uint64_t hash_murmur2_64(const void *key,
                         size_t len,
                         uint64_t seed);

#endif //CETECH_HASH_H
