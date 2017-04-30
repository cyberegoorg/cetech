
#include <string.h>

#include "cetech/array.inl"


//==============================================================================
// Interface
//==============================================================================

size_t strlen(const char *s) {
    CEL_ASSERT("string", s != NULL);

    return strlen(s);
}

char *str_dup(const char *s,
                 struct allocator *allocator) {
    char *d = (char *) CEL_ALLOCATE(allocator, char, strlen(s) + 1);
    CEL_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

int strcmp(const char *s1,
               const char *s2) {
    CEL_ASSERT("string", s1 != NULL);
    CEL_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}

void str_set(char *__restrict result,
                 const char *__restrict str) {
    memory_copy(result, str, strlen(str));
}

int str_startswith(const char *string,
                       const char *with) {
    return !strncmp(string, with, strlen(with));
}