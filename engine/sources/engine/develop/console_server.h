#ifndef CETECH_CONSOLE_SERVER_API_H
#define CETECH_CONSOLE_SERVER_API_H

#include "include/mpack/mpack.h"

//==============================================================================
// Interface
//==============================================================================

//==============================================================================
// Conosle Server
//==============================================================================

typedef int (*console_server_command_t)(mpack_node_t,
                                        mpack_writer_t *);

struct ConsoleServerApiV1 {
    void (*consolesrv_push_begin)();

    void (*consolesrv_register_command)(const char *,
                                        console_server_command_t);
};

#endif //CETECH_CONSOLE_SERVER_API_H
