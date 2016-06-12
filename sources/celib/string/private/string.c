#include <memory.h>
#include <stdlib.h>

#include "../../errors/errors.h"
#include "../../memory/memory.h"
#include "../../memory/allocator.h"

size_t string_lenght(const char *s) {
    return strlen(s);
}

char *string_duplicate(const char *s, Alloc_t allocator) {
    char *d = alloc_alloc(allocator, string_lenght(s) + 1);
    CE_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

int string_compare(const char *s1, const char* s2) {
    CE_ASSERT("string", s1 != NULL);
    CE_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}