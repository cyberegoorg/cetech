#ifndef CETECH_STACKTRACE_H
#define CETECH_STACKTRACE_H

char *os_stacktrace(int skip);

void os_stacktrace_free(char *st);

#endif //CETECH_STACKTRACE_H
