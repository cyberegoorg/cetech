//==============================================================================
// Based on bitsquid foundation.
// git+web: https://bitbucket.org/bitsquid/foundation
//==============================================================================

#ifndef CELIB_MEMORY_H
#define CELIB_MEMORY_H

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>
#include <stdint.h>


//==============================================================================
// Memory
//==============================================================================

void *cel_malloc(size_t size);
void cel_free(void *ptr);

void *memory_copy(void *__restrict dest,
                  const void *__restrict src,
                  size_t n);

void *memory_set(void *__restrict dest,
                 int c,
                 size_t n);

const void *pointer_align_forward(const void *p,
                                  uint32_t align);


const void *pointer_add(const void *p,
                        uint32_t bytes);

const void *pointer_sub(const void *p,
                        uint32_t bytes);

#endif //CELIB_MEMORY_H
