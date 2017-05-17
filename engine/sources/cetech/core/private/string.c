
#include <string.h>

#include <cetech/core/array.inl>


//==============================================================================
// Interface
//==============================================================================

size_t str_len(const char *s) {
    CETECH_ASSERT("string", s != NULL);

    return strlen(s);
}

char *str_dup(const char *s,
              struct allocator *allocator) {
    char *d = (char *) CETECH_ALLOCATE(allocator, char, strlen(s) + 1);
    CETECH_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

int str_cmp(const char *s1,
            const char *s2) {
    CETECH_ASSERT("string", s1 != NULL);
    CETECH_ASSERT("string", s2 != NULL);

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