#ifndef CETECH_STRING_H
#define CETECH_STRING_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/containers/array.h>
#include "celib/types.h"
#include "celib/memory/types.h"


typedef ARRAY_T(char) dynstr_t;
#define DYNSTR_INIT(a, alloc) ARRAY_INIT(char, a, alloc)
#define DYNSTR_CAT(a, str) ARRAY_PUSH(char, a, str, strlen(str))

//==============================================================================
// Interface
//==============================================================================

size_t str_lenght(const char *s);

char *str_duplicate(const char *s,
                    struct allocator *allocator);

int str_compare(const char *s1,
                const char *s2);

void str_set(char *result,
             const char *str);

int str_startswith(const char **string,
                   const char **with);

#endif //CETECH_STRING_H
