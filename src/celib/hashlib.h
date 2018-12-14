#ifndef CE_HASH_H
#define CE_HASH_H

#include <stdint.h>
#include <celib/module.inl>

#define CE_ID64_0(str, hash) hash
//#define CE_ID32_0(str) ce_id_a0->id32_from_str(str)

struct ce_id_a0 {
    uint64_t (*id64)(const char *str);

//    uint32_t (*id32_from_str)(const char *str);

    const char *(*str_from_id64)(uint64_t key);

    const char *(*str_from_id32)(uint32_t key);
};

CE_MODULE(ce_id_a0);

#endif //CE_HASH_H
