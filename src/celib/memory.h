#ifndef CE_MEMSYS_H
#define CE_MEMSYS_H

#include <celib/module.inl>

#define CE_MEMORY_API \
    CE_ID64_0("ce_memory_a0", 0x5fc4626a7b1e5fdfULL)

struct ce_alloc;

//! Memory system API V0
struct ce_memory_a0 {
    struct ce_alloc *system;

    char *(*str_dup)(const char *s,
                     struct ce_alloc *allocator);
};

CE_MODULE(ce_memory_a0);

#endif //CE_MEMSYS_H
