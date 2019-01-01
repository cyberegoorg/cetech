#ifndef CE_ID_H
#define CE_ID_H

#include <stdint.h>
#include <celib/module.inl>

#define CE_ID_API \
    CE_ID64_0("ce_id_a0", 0x1784034d025c1467ULL)

#define CE_ID64_0(str, hash) hash

struct ce_id_a0 {
    uint64_t (*id64)(const char *str);
    const char *(*str_from_id64)(uint64_t key);
};

CE_MODULE(ce_id_a0);

#endif //CE_ID_H
