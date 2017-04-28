
#include <string.h>

#include "celib/array.inl"


//==============================================================================
// Interface
//==============================================================================

size_t cel_strlen(const char *s) {
    CEL_ASSERT("string", s != NULL);

    return strlen(s);
}

char *cel_strdup(const char *s,
                 struct cel_allocator *allocator) {
    char *d = (char *) CEL_ALLOCATE(allocator, char, cel_strlen(s) + 1);
    CEL_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

int cel_strcmp(const char *s1,
               const char *s2) {
    CEL_ASSERT("string", s1 != NULL);
    CEL_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}

void cel_str_set(char *__restrict result,
                 const char *__restrict str) {
    memory_copy(result, str, cel_strlen(str));
}

int cel_str_startswith(const char *string,
                       const char *with) {
    return !strncmp(string, with, cel_strlen(with));
}