#ifndef CETECH_STACKTRACE_H
#define CETECH_STACKTRACE_H

//==============================================================================
// Interface
//==============================================================================

char *utils_stacktrace(int skip);
void utils_stacktrace_free(char *st);

#endif //CETECH_STACKTRACE_H
