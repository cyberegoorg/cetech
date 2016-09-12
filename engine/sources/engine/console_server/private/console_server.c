//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <include/mpack/mpack.h>

#include "include/nanomsg/nn.h"
#include "include/nanomsg/reqrep.h"
#include "include/nanomsg/pubsub.h"

#include "engine/console_server/console_server.h"
#include "celib/errors/errors.h"
#include "celib/string/string.h"

#include "engine/config_system/config_system.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "console_server"

#define MAX_COMMANDS 256
#define MAX_COMMAND_NAME_LEN 128

//==============================================================================
// Globals
//==============================================================================

#define _G ConsoleServerGlobals

static struct G {
    char name[MAX_COMMANDS][MAX_COMMAND_NAME_LEN];
    console_server_command_t commands[MAX_COMMANDS];

    int rpc_socket;
    int log_socket;
} ConsoleServerGlobals = {0};


//==============================================================================
// Private
//==============================================================================

static console_server_command_t _find_command(const char *name) {
    for (int i = 1; i < MAX_COMMANDS; ++i) {
        if (_G.name[i][0] == '\0' || str_compare(_G.name[i], name) != 0) {
            continue;
        }

        return _G.commands[i];
    }

    return 0;
}

static void _serve_command(const char *packet,
                           int len) {
    mpack_tree_t tree;
    mpack_tree_init(&tree, packet, (size_t) len);
    mpack_tree_parse(&tree);
    mpack_error_t errort = mpack_tree_error(&tree);

    if (errort != mpack_ok) {
        log_error(LOG_WHERE, "%s", mpack_error_to_string(errort));
        return;
    }

    mpack_node_t root = mpack_tree_root(&tree);

    // Find command
    char cmd_name[256] = {0};
    mpack_node_t name_node = mpack_node_map_cstr(root, "name");
    mpack_node_copy_cstr(name_node, cmd_name, 256);

    console_server_command_t rpc_command = _find_command(cmd_name);

    char *data;
    size_t size;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &data, &size);

    mpack_start_map(&writer, 1);
    mpack_write_cstr(&writer, "response");

    int ret_n = 0;
    if (rpc_command != NULL) {
        //log_debug(LOG_WHERE, "call: %s", cmd_name);
        mpack_node_t args = mpack_node_map_cstr(root, "args");
        ret_n = rpc_command(args, &writer);
    }

    if (!ret_n) {
        mpack_write_nil(&writer);
    }

    mpack_finish_map(&writer);
    CE_ASSERT(LOG_WHERE, mpack_writer_destroy(&writer) == mpack_ok);

    int bytes = nn_send(_G.rpc_socket, data, size, 0);
    CE_ASSERT(LOG_WHERE, (size_t) bytes == size);

}

//==============================================================================
// Interface
//==============================================================================

int consolesrv_init() {
    log_debug(LOG_WHERE, "Init");

    cvar_t rpc_port = config_new_int("console_server.rpc.port", "Console server rpc port", 4444);
    cvar_t rpc_addr = config_new_string("console_server.rpc.addr", "Console server rpc addr", "ws://*");

    int socket = nn_socket(AF_SP, NN_REP);
    if (socket < 0) {
        log_error(LOG_WHERE, "Could not create nanomsg socket: %s", nn_strerror(errno));
        return 0;
    }

    char addr[128] = {0};
    snprintf(addr, 128, "%s:%d", config_get_string(rpc_addr), config_get_int(rpc_port));

    log_debug(LOG_WHERE, "RPC address: %s", addr);

    if (nn_bind(socket, addr) < 0) {
        log_error(LOG_WHERE, "Could not bind socket to '%s': %s", addr, nn_strerror(errno));
        return 0;
    }

    _G.rpc_socket = socket;
////
    socket = nn_socket(AF_SP, NN_PUB);
    if (socket < 0) {
        log_error(LOG_WHERE, "Could not create nanomsg socket: %s", nn_strerror(errno));
        return 0;
    }

    if (nn_bind(socket, "ws://*:4445") < 0) {
        log_error(LOG_WHERE, "Could not bind socket to '%s': %s", addr, nn_strerror(errno));
        return 0;
    }
    _G.log_socket = socket;

    log_register_handler(nano_log_handler, &_G.log_socket);

    return 1;
}

void consolesrv_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");
}

void consolesrv_register_command(const char *name,
                                 console_server_command_t cmd) {
    for (int i = 1; i < MAX_COMMANDS; ++i) {
        if (_G.name[i][0] != '\0') {
            continue;
        }

        str_set(_G.name[i], name);
        _G.commands[i] = cmd;
    }
}

void consolesrv_update() {
    char *buf = NULL;

    int max_iteration = 100;
    int bytes = 0;
    while (--max_iteration) {
        bytes = nn_recv(_G.rpc_socket, &buf, NN_MSG, NN_DONTWAIT);

        if (bytes <= 0) {
            break;
        }

        _serve_command(buf, bytes);
        nn_freemsg(buf);
    }
}
