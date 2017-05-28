//! \defgroup Memory
//! Memory system
//! \{

#ifndef CETECH_MEMSYS_H
#define CETECH_MEMSYS_H

//==============================================================================
// Api
//==============================================================================

//! Memory system API V0
struct memory_api_v0 {
    //! Main allcator
    //! \return Main alocator
    struct allocator *(*main_allocator)();

    //! Main scratch allocator
    //! \return Main scratch alocator
    struct allocator *(*main_scratch_allocator)();

    char *(*str_dup)(const char *s,
                  struct allocator *allocator);
};

#endif //CETECH_MEMSYS_H

//! \}
