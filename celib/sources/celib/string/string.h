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

static size_t str_lenght(const char *s) {
    CE_ASSERT("string", s != NULL);

    return strlen(s);
}

static char *str_duplicate(const char *s,
                           struct allocator *allocator) {
    char *d = (char *) CE_ALLOCATE(allocator, char, str_lenght(s) + 1);
    CE_ASSERT("string", d != NULL);

    if (d == NULL) return NULL;

    strcpy(d, s);

    return d;
}

static int str_compare(const char *s1,
                       const char *s2) {
    CE_ASSERT("string", s1 != NULL);
    CE_ASSERT("string", s2 != NULL);

    return strcmp(s1, s2);
}

static void str_set(char *result,
                    const char *__restrict str) {
    memory_copy(result, str, str_lenght(str));
}

static int str_startswith(const char *string,
                          const char *with) {
    return !strncmp(string, with, str_lenght(with));
}

#endif //CELIB_STRING_H
