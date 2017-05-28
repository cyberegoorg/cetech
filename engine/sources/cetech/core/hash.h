#ifndef CETECH_HASH_H
#define CETECH_HASH_H

//==============================================================================
// Include
//==============================================================================

#include <stdint.h>

//==============================================================================
// Interface
//==============================================================================

struct hash_api_v0 {
    //! 64bit murmur 2 hash
    //! \param key Key
    //! \param seed Seed
    //! \return Hashed value
    uint64_t (*hash_murmur2_64)(const void *key,
                             uint64_t len,
                             uint64_t seed);

    uint64_t (*id64_from_str)(const char *str);
};

#endif //CETECH_HASH_H
