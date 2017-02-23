//! \defgroup Memory
//! Memory system
//! \{

#ifndef CELIB_MEMSYS_H
#define CELIB_MEMSYS_H

#include "celib/memory/types.h"

//! Init memory system
//! \param scratch_buffer_size Main scratch buffer size
void memsys_init(int scratch_buffer_size);

//! Shutdown memory system
void memsys_shutdown();

//! Main allcator
//! \return Main alocator
struct cel_allocator *_memsys_main_allocator();

//! Main scratch allocator
//! \return Main scratch alocator
struct cel_allocator *_memsys_main_scratch_allocator();


//! Memory system API V0
struct MemSysApiV0 {
    //! Main allcator
    //! \return Main alocator
    struct cel_allocator *(*main_allocator)();

    //! Main scratch allocator
    //! \return Main scratch alocator
    struct cel_allocator *(*main_scratch_allocator)();
};


#endif //CELIB_MEMSYS_H

//! \}
