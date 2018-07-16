#ifndef CETECH_HASH_H
#define CETECH_HASH_H



//==============================================================================
// Include
//==============================================================================

#include <stdint.h>
#include <corelib/module.inl>


//==============================================================================
// Api
//==============================================================================

#define CT_ID64_0(str, hash) hash
//#define CT_ID32_0(str) ct_hashlib_a0->id32_from_str(str)

struct ct_hashlib_a0 {
    uint64_t (*id64)(const char *str);

//    uint32_t (*id32_from_str)(const char *str);

    const char *(*str_from_id64)(uint64_t key);

    const char *(*str_from_id32)(uint32_t key);
};

CT_MODULE(ct_hashlib_a0);

#endif //CETECH_HASH_H
