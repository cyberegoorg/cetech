//! \defgroup Memory
//! Memory system
//! \{

#ifndef CETECH_MEMSYS_H
#define CETECH_MEMSYS_H

#ifdef __cplusplus
extern "C" {
#endif

struct ct_core_allocator_a0 {
    struct cel_alloc *(*get_allocator)();
};

//==============================================================================
// Api
//==============================================================================

//! Memory system API V0
struct ct_memory_a0 {

    //! Main allcator
    //! \return Main alocator
    struct cel_alloc *(*main_allocator)();

    //! Main scratch allocator
    //! \return Main scratch alocator
    struct cel_alloc *(*main_scratch_allocator)();

    char *(*str_dup)(const char *s,
                     struct cel_alloc *allocator);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MEMSYS_H

//! \}
