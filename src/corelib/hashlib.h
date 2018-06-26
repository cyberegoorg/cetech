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

#define CT_ID64_0(str) ct_hashlib_a0->id64_from_str(str)
#define CT_ID32_0(str) ct_hashlib_a0->id32_from_str(str)

#define CT_STR64_0(k) ct_hashlib_a0->str_from_id64(k)
#define CT_STR32_0(k) ct_hashlib_a0->str_from_id32(k)

struct ct_hashlib_a0 {
    uint64_t (*id64_from_str)(const char *str);

    uint32_t (*id32_from_str)(const char *str);

    const char *(*str_from_id64)(uint64_t key);

    const char *(*str_from_id32)(uint32_t key);
};

CT_MODULE(ct_hashlib_a0);

#endif //CETECH_HASH_H
