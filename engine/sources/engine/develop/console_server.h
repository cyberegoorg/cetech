#ifndef CETECH_CONSOLE_SERVER_API_H
#define CETECH_CONSOLE_SERVER_API_H

#include "types.h"

//==============================================================================
// Interface
//==============================================================================

void consolesrv_push_begin();

void consolesrv_register_command(const char *,
                                 console_server_command_t);

#endif //CETECH_CONSOLE_SERVER_API_H
