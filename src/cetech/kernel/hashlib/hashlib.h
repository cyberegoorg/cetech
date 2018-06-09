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

#define CT_STR64_0(k) ct_hashlib_a0.str_from_id64(k)
#define CT_STR32_0(k) ct_hashlib_a0.str_from_id32(k)

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

    const char* (*str_from_id64)(uint64_t key);
    const char* (*str_from_id32)(uint32_t key);
};


#endif //CETECH_HASH_H
