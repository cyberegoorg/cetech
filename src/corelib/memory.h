#ifndef CETECH_MEMSYS_H
#define CETECH_MEMSYS_H

#include <corelib/module.inl>

struct ct_alloc;

struct ct_core_allocator_a0 {
    struct ct_alloc *(*get_allocator)();
};

//==============================================================================
// Api
//==============================================================================

//! Memory system API V0
struct ct_memory_a0 {

    //! Main allcator
    //! \return Main alocator
    struct ct_alloc *(*main_allocator)();


    char *(*str_dup)(const char *s,
                     struct ct_alloc *allocator);
};

CT_MODULE(ct_memory_a0);

#endif //CETECH_MEMSYS_H
