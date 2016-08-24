#include <memory.h>
#include "celib/errors/errors.h"

#define LOG_WHERE "os.memory"

void *os_malloc(size_t size) {
    CE_ASSERT(LOG_WHERE, size > 0);

    void *mem = malloc(size);
    if(mem == NULL) {
        log_error(LOG_WHERE, "Malloc return NULL");
        return NULL;
    }

    return mem;
}

void os_free(void *ptr) {
    free(ptr);
}
