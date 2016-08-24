#ifndef CETECH_OS_MEMORY_H
#define CETECH_OS_MEMORY_H

//==============================================================================
// Interface
//==============================================================================

void *os_malloc(size_t size);
void os_free(void *ptr);

#endif //CETECH_OS_MEMORY_H