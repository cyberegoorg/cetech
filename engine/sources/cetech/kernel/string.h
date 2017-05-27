#ifndef CETECH_STRING_H
#define CETECH_STRING_H

#include <stddef.h>

struct allocator;

//==============================================================================
// Interface
//==============================================================================

char *str_dup(const char *s,
              struct allocator *allocator);

int str_startswith(const char *string,
                   const char *with);

#endif //CETECH_STRING_H
