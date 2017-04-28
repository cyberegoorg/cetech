#ifndef CELIB_STRING_H
#define CELIB_STRING_H

//==============================================================================
// Interface
//==============================================================================

size_t cel_strlen(const char *s);

char *cel_strdup(const char *s,
                 struct cel_allocator *allocator);

int cel_strcmp(const char *s1,
               const char *s2);

void cel_str_set(char *__restrict result,
                 const char *__restrict str);

int cel_str_startswith(const char *string,
                       const char *with);

#endif //CELIB_STRING_H
