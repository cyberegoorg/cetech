//! \defgroup Memory
//! Memory system
//! \{

#ifndef CETECH_MEMSYS_H
#define CETECH_MEMSYS_H

//==============================================================================
// Includes
//==============================================================================

struct api_v0;

enum {
    MEMORY_API_ID = 14,
};

//==============================================================================
// Interface
//==============================================================================

//! Init memory system
//! \param scratch_buffer_size Main scratch buffer size
void memsys_init(int scratch_buffer_size);

void memsys_init_api(struct api_v0* api);

//! Shutdown memory system
void memsys_shutdown();

//! Main allcator
//! \return Main alocator
struct allocator *_memsys_main_allocator();

//! Main scratch allocator
//! \return Main scratch alocator
struct allocator *_memsys_main_scratch_allocator();

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
};


#endif //CETECH_MEMSYS_H

//! \}
