#ifndef CELIB_STRING_H
#define CELIB_STRING_H

//==============================================================================
// Includes
//==============================================================================

#include <string.h>

#include "../containers/array.h"
#include "../types.h"
#include "../memory/types.h"
#include "../errors/errors.h"

typedef ARRAY_T(char) dynstr_t;
#define DYNSTR_INIT(a, alloc) ARRAY_INIT(char, a, alloc)
#define DYNSTR_CAT(a, str) ARRAY_PUSH(char, a, str, strlen(str))

//==============================================================================
// Interface
//==============================================================================

static size_t cel_strlen(const char *s) {
    CEL_ASSERT("string", s != NULL);

    return strlen(s);
}

static char *cel_strdup(const char *s,
                        struct cel_allocator *allocator) {
    char *d = (char *) CEL_ALLOCATE(allocator, char, cel_strlen(s) + 1);
    CEL_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

static int cel_strcmp(const char *s1,
                      const char *s2) {
    CEL_ASSERT("string", s1 != NULL);
    CEL_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}

static void cel_str_set(char *__restrict result,
                        const char *__restrict str) {
    memory_copy(result, str, cel_strlen(str));
}

static int cel_str_startswith(const char *string,
                              const char *with) {
    return !strncmp(string, with, cel_strlen(with));
}

#endif //CELIB_STRING_H
