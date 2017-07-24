#ifndef CETECH_PROCESS_H
#define CETECH_PROCESS_H


#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Process
//==============================================================================

struct ct_process_a0 {
    int (*exec)(const char *argv);
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_PROCESS_H
