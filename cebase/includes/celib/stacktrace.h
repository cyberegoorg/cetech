#ifndef CELIB_STACKTRACE_H
#define CELIB_STACKTRACE_H

char *cel_stacktrace(int skip);

void cel_stacktrace_free(char *st);

#endif //CELIB_STACKTRACE_H
