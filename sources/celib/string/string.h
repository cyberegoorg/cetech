#ifndef CETECH_STRING_H
#define CETECH_STRING_H

#include "../memory/allocator.h"

char *str_duplicate(const char *s,
                    Alloc_t allocator);

int str_compare(const char *s1, const char *s2);

size_t str_lenght(const char *s);

#endif //CETECH_STRING_H
