#include <memory.h>
#include <stdlib.h>

#include "../../errors/errors.h"
#include "../../memory/memory.h"
#include "../../memory/allocator.h"

size_t str_lenght(const char *s) {
    return strlen(s);
}

char *str_duplicate(const char *s, Alloc_t allocator) {
    char *d = alloc_alloc(allocator, str_lenght(s) + 1);
    CE_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

int str_compare(const char *s1, const char *s2) {
    CE_ASSERT("string", s1 != NULL);
    CE_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}