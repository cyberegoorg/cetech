#ifndef CETECH_STACKTRACE_H
#define CETECH_STACKTRACE_H

char *machine_stacktrace(int skip);

void machine_stacktrace_free(char *st);

#endif //CETECH_STACKTRACE_H
