#ifndef CETECH_VIRT_SYSTEM_INL
#define CETECH_VIRT_SYSTEM_INL

#include <sys/mman.h>

static void *_reallocate_virt(const ce_alloc_o0 *a,
                              void *ptr,
                              size_t size,
                              size_t old_size,
                              size_t align,
                              const char *filename,
                              uint32_t line) {

    if(!size)  {
        munmap(ptr, old_size);
        return NULL;
    }

    return mmap(NULL,
                size,
                PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS |
                MAP_NORESERVE,
                -1, 0);
}

static struct ce_alloc_vt0 virt_system_vt = {
        .reallocate = _reallocate_virt
};


static struct ce_alloc_t0 _virt_system_allocator = {
        .inst = NULL,
        .vt = &virt_system_vt,
};


#endif //CETECH_VIRT_SYSTEM_INL
