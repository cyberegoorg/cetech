#ifndef CETECH_STRING_H
#define CETECH_STRING_H

#include <stddef.h>

//==============================================================================
// Interface
//==============================================================================

size_t str_len(const char *s);

char *str_dup(const char *s,
                 struct allocator *allocator);

int str_cmp(const char *s1,
               const char *s2);

void str_set(char *__restrict result,
                 const char *__restrict str);

int str_startswith(const char *string,
                       const char *with);

#endif //CETECH_STRING_H
