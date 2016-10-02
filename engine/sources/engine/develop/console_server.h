#ifndef CETECH_CONSOLE_SERVER_API_H
#define CETECH_CONSOLE_SERVER_API_H

//==============================================================================
// Typedefs
//==============================================================================


#include "include/mpack/mpack.h"

typedef int (*console_server_command_t)(mpack_node_t,
                                        mpack_writer_t *);


//==============================================================================
// Interface
//==============================================================================

int consolesrv_init(int stage);

void consolesrv_shutdown();

void consolesrv_update();

void consolesrv_push_begin();

void consolesrv_register_command(const char *,
                                 console_server_command_t);

#endif //CETECH_CONSOLE_SERVER_API_H
