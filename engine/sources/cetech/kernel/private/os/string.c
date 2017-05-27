
#include <string.h>

#include <cetech/core/array.inl>


//==============================================================================
// Interface
//==============================================================================

char *str_dup(const char *s,
              struct allocator *allocator) {
    char *d = (char *) CETECH_ALLOCATE(allocator, char, strlen(s) + 1);
    CETECH_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}


int str_startswith(const char *string,
                   const char *with) {
    return !strncmp(string, with, strlen(with));
}