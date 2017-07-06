//! \defgroup Memory
//! Memory system
//! \{

#ifndef CETECH_MEMSYS_H
#define CETECH_MEMSYS_H

#ifdef __cplusplus
extern "C" {
#endif

struct ct_core_allocator_api_v0 {
    struct ct_allocator* (*get_allocator)();
};

//==============================================================================
// Api
//==============================================================================

//! Memory system API V0
struct ct_memory_api_v0 {
    //! Main allcator
    //! \return Main alocator
    struct ct_allocator *(*main_allocator)();

    //! Main scratch allocator
    //! \return Main scratch alocator
    struct ct_allocator *(*main_scratch_allocator)();

    char *(*str_dup)(const char *s,
                     struct ct_allocator *allocator);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MEMSYS_H

//! \}
