#include <string.h>

#include <celib/errors/errors.h>
#include <celib/memory/memory.h>
#include "../../os/memory.h"

size_t str_lenght(const char *s) {
    CE_ASSERT("string", s != NULL);

    return strlen(s);
}

char* str_duplicate(const char* s, struct allocator *allocator) {
    char *d = (char *) CE_ALLOCATE(allocator, char, str_lenght(s) + 1);
    CE_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

int str_compare(const char* s1, const char* s2) {
    CE_ASSERT("string", s1 != NULL);
    CE_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}

void str_set(char* result, const char* __restrict str) {
    memory_copy(result, str, str_lenght(str));
}

int str_startswith(const char* string, const char* with) {
    return !strncmp(string, with, str_lenght(with));
}