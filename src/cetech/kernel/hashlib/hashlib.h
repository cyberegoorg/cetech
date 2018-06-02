#ifndef CETECH_HASH_H
#define CETECH_HASH_H



//==============================================================================
// Include
//==============================================================================

#include <stdint.h>


//==============================================================================
// Api
//==============================================================================

#define CT_ID64_0(str) ct_hashlib_a0.id64_from_str(str)
#define CT_ID32_0(str) ct_hashlib_a0.id32_from_str(str)

struct ct_hashlib_a0 {

    //! 64bit murmur 2 hash
    //! \param key Key
    //! \param seed Seed
    //! \return Hashed value
    uint64_t (*hash_murmur2_64)(const void *key,
                                uint64_t len,
                                uint64_t seed);

    uint32_t (*hash_murmur2_32)(const void *key,
                                uint32_t len,
                                uint32_t seed);

    uint64_t (*id64_from_str)(const char *str);
    uint32_t (*id32_from_str)(const char *str);
};


#endif //CETECH_HASH_H
