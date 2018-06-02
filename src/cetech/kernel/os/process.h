#ifndef CETECH_PROCESS_H
#define CETECH_PROCESS_H




//==============================================================================
// Process
//==============================================================================

struct ct_process_a0 {
    int (*exec)(const char *argv);
};



#endif //CETECH_PROCESS_H
