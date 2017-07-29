//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <cetech/engine/console_server/console_server.h>

#include "cetech/core/api/api_system.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/log/log.h"
#include "celib/map.inl"
#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/os/errors.h"
#include "cetech/core/module/module.h"

#include "include/mpack/mpack.h"
#include "include/nanomsg/nn.h"
#include "include/nanomsg/reqrep.h"
#include "include/nanomsg/pubsub.h"
#include "include/nanomsg/pipeline.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "console_server"


//==============================================================================
// Private
//==============================================================================

namespace {
    static struct ConsoleServerGlobals {
        Map<ct_console_srv_command_t> commands;

        int rpc_socket;
        int log_socket;
        int push_socket;

        ct_cvar cv_rpc_addr;
        ct_cvar cv_log_addr;
        ct_cvar cv_push_addr;
    } _G;

    static ct_console_srv_command_t find_command(const char *name) {
        uint64_t key = ct_hash_a0.id64_from_str(name);

        auto command = map::get<ct_console_srv_command_t>(_G.commands, key,
                                                          nullptr);

        if (!command) {
            ct_log_a0.error(LOG_WHERE, "Invalid command \"%s\"", name);
        }

        return command;
    }

    static void serve_command(const char *packet,
                              int len) {
        mpack_tree_t tree;
        mpack_tree_init(&tree, packet, (size_t) len);
        mpack_tree_parse(&tree);
        mpack_error_t errort = mpack_tree_error(&tree);

        if (errort != mpack_ok) {
            ct_log_a0.error(LOG_WHERE, "%s", mpack_error_to_string(errort));
            return;
        }

        mpack_node_t root = mpack_tree_root(&tree);

        // Find command
        char cmd_name[256] = {};
        mpack_node_t name_node = mpack_node_map_cstr(root, "name");
        mpack_node_copy_cstr(name_node, cmd_name, 256);

        ct_console_srv_command_t rpc_command = find_command(cmd_name);

        mpack_node_t id_node = mpack_node_map_cstr(root, "id");
        double id = mpack_node_double(id_node);

        char *data;
        size_t size;
        mpack_writer_t writer;
        mpack_writer_init_growable(&writer, &data, &size);

        mpack_start_map(&writer, 2);

        mpack_write_cstr(&writer, "id");
        mpack_write_double(&writer, id);

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
        CETECH_ASSERT(LOG_WHERE, mpack_writer_destroy(&writer) == mpack_ok);

        int bytes = nn_send(_G.rpc_socket, data, size, 0);
        CETECH_ASSERT(LOG_WHERE, (size_t) bytes == size);
        free(data);
    }

    static int cmd_ready(mpack_node_t args,
                         mpack_writer_t *writer) {
        CEL_UNUSED(args, writer);
        return 0;
    }
}

//==============================================================================
// Interface
//==============================================================================

namespace consoleserver {

    void register_command(const char *name,
                          ct_console_srv_command_t cmd) {

        uint64_t key = ct_hash_a0.id64_from_str(name);
        map::set(_G.commands, key, cmd);
    }

    void push_begin() {
        if (_G.push_socket) {
            nn_send(_G.push_socket, "begin", strlen("begin") + 1, 0);
        }
    }

    static void update() {
        char *buf = NULL;

        int max_iteration = 100;
        int bytes = 0;
        while (--max_iteration) {
            bytes = nn_recv(_G.rpc_socket, &buf, NN_MSG, NN_DONTWAIT);

            if (bytes <= 0) {
                break;
            }

            serve_command(buf, bytes);
            nn_freemsg(buf);
        }
    }

}

namespace consoleserver_module {
    static ct_console_srv_a0 console_api = {
            .push_begin = consoleserver::push_begin,
            .register_command = consoleserver::register_command,
            .update = consoleserver::update
    };

    static void _init_cvar(struct ct_config_a0 config) {
        _G = {};

        _G.cv_rpc_addr = config.new_str("develop.rpc.addr",
                                        "Console server rpc addr",
                                        "ws://*:4444");

        _G.cv_log_addr = config.new_str("develop.logsystem.addr",
                                        "Console server logsystem addr",
                                        "ws://*:4445");

        _G.cv_push_addr = config.new_str("develop.push.addr", "Push addr", "");
    }

    static void _init_api(ct_api_a0 *api) {
        api->register_api("ct_console_srv_a0", &console_api);
    }

    static void _init(ct_api_a0 *api) {
        _init_api(api);


        _init_cvar(ct_config_a0);

        _G.commands.init(ct_memory_a0.main_allocator());

        const char *addr = 0;

        ct_log_a0.debug(LOG_WHERE, "Init");

        int socket = nn_socket(AF_SP, NN_REP);
        if (socket < 0) {
            ct_log_a0.error(LOG_WHERE, "Could not create nanomsg socket: %s",
                            nn_strerror(errno));
            return;// 0;
        }
        addr = ct_config_a0.get_string(_G.cv_rpc_addr);

        ct_log_a0.debug(LOG_WHERE, "RPC address: %s", addr);

        if (nn_bind(socket, addr) < 0) {
            ct_log_a0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                            addr,
                            nn_strerror(errno));
            return;// 0;
        }

        _G.rpc_socket = socket;
////

        if (ct_config_a0.get_string(_G.cv_push_addr)[0] != '\0') {
            socket = nn_socket(AF_SP, NN_PUSH);
            if (socket < 0) {
                ct_log_a0.error(LOG_WHERE,
                                "Could not create nanomsg socket: %s",
                                nn_strerror(errno));
                return;// 0;
            }

            addr = ct_config_a0.get_string(_G.cv_push_addr);

            ct_log_a0.debug(LOG_WHERE, "Push address: %s", addr);

            if (nn_connect(socket, addr) < 0) {
                ct_log_a0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                                addr,
                                nn_strerror(errno));
                return;// 0;
            }
            _G.push_socket = socket;
        }


////
        socket = nn_socket(AF_SP, NN_PUB);
        if (socket < 0) {
            ct_log_a0.error(LOG_WHERE, "Could not create nanomsg socket: %s",
                            nn_strerror(errno));
            return;// 0;
        }

        addr = ct_config_a0.get_string(_G.cv_log_addr);

        ct_log_a0.debug(LOG_WHERE, "LOG address: %s", addr);

        if (nn_bind(socket, addr) < 0) {
            ct_log_a0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                            addr,
                            nn_strerror(errno));
            return;// 0;
        }
        _G.log_socket = socket;

        ct_log_a0.register_handler(ct_nano_log_handler, &_G.log_socket);

        consoleserver::register_command("console_server.ready", cmd_ready);
    }


    static void _shutdown() {
        ct_log_a0.debug(LOG_WHERE, "Shutdown");

        nn_close(_G.push_socket);
        //nn_close(_G.log_socket);
        nn_close(_G.rpc_socket);

        _G.commands.destroy();
    }

}


CETECH_MODULE_DEF(
        consoleserver,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
        },
        {
            consoleserver_module::_init(api);
        },
        {
            CEL_UNUSED(api);
            consoleserver_module::_shutdown();
        }
)

