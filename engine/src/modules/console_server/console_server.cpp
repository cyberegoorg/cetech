//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include <cetech/core/api.h>
#include <cetech/core/config.h>
#include <cetech/core/module.h>
#include <cetech/core/memory.h>
#include <cetech/core/log.h>
#include <cetech/celib/map2.inl>
#include <cetech/modules/resource.h>
#include <cetech/modules/develop.h>
#include <cetech/core/hash.h>
#include <cetech/modules/console_server.h>
#include <cetech/core/errors.h>

#include "include/mpack/mpack.h"
#include "include/nanomsg/nn.h"
#include "include/nanomsg/reqrep.h"
#include "include/nanomsg/pubsub.h"
#include "include/nanomsg/pipeline.h"

IMPORT_API(memory_api_v0);
IMPORT_API(config_api_v0);
IMPORT_API(log_api_v0);
IMPORT_API(hash_api_v0);

using namespace cetech;

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "console_server"


//==============================================================================
// Private
//==============================================================================

namespace {
    static struct ConsoleServerGlobals {
        Map<console_server_command_t> commands;

        int rpc_socket;
        int log_socket;
        int push_socket;

        cvar_t cv_rpc_addr;
        cvar_t cv_log_addr;
        cvar_t cv_push_addr;
    } _G = {0};

    static console_server_command_t find_command(const char *name) {
        uint64_t key = hash_api_v0.id64_from_str(name);

        auto command = map::get<console_server_command_t>(_G.commands, key,
                                                          nullptr);

        if (!command) {
            log_api_v0.error(LOG_WHERE, "Invalid command \"%s\"", name);
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
            log_api_v0.error(LOG_WHERE, "%s", mpack_error_to_string(errort));
            return;
        }

        mpack_node_t root = mpack_tree_root(&tree);

        // Find command
        char cmd_name[256] = {0};
        mpack_node_t name_node = mpack_node_map_cstr(root, "name");
        mpack_node_copy_cstr(name_node, cmd_name, 256);

        console_server_command_t rpc_command = find_command(cmd_name);

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
        return 0;
    }
}

//==============================================================================
// Interface
//==============================================================================

namespace consoleserver {

    void register_command(const char *name,
                          console_server_command_t cmd) {

        uint64_t key = hash_api_v0.id64_from_str(name);
        map::set(_G.commands, key, cmd);
    }

    void push_begin() {
        if (_G.push_socket) {
            nn_send(_G.push_socket, "begin", strlen("begin") + 1, 0);
        }
    }

}

namespace consoleserver_module {
    static struct cnsole_srv_api_v0 console_api = {
            .consolesrv_push_begin = consoleserver::push_begin,
            .consolesrv_register_command = consoleserver::register_command
    };

    static void _init_cvar(struct config_api_v0 config) {
        _G = {0};

        _G.cv_rpc_addr = config.new_str("develop.rpc.addr",
                                        "Console server rpc addr",
                                        "ws://*:4444");

        _G.cv_log_addr = config.new_str("develop.log.addr",
                                        "Console server log addr",
                                        "ws://*:4445");

        _G.cv_push_addr = config.new_str("develop.push.addr", "Push addr", "");
    }

    static void _init_api(struct api_v0 *api) {
        api->register_api("cnsole_srv_api_v0", &console_api);
    }

    static void _init(struct api_v0 *api) {
        GET_API(api, memory_api_v0);
        GET_API(api, config_api_v0);
        GET_API(api, log_api_v0);
        GET_API(api, hash_api_v0);

        _G.commands.init(memory_api_v0.main_allocator());

        const char *addr = 0;

        log_api_v0.debug(LOG_WHERE, "Init");

        int socket = nn_socket(AF_SP, NN_REP);
        if (socket < 0) {
            log_api_v0.error(LOG_WHERE, "Could not create nanomsg socket: %s",
                             nn_strerror(errno));
            return;// 0;
        }
        addr = config_api_v0.get_string(_G.cv_rpc_addr);

        log_api_v0.debug(LOG_WHERE, "RPC address: %s", addr);

        if (nn_bind(socket, addr) < 0) {
            log_api_v0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                             addr,
                             nn_strerror(errno));
            return;// 0;
        }

        _G.rpc_socket = socket;
////

        if (config_api_v0.get_string(_G.cv_push_addr)[0] != '\0') {
            socket = nn_socket(AF_SP, NN_PUSH);
            if (socket < 0) {
                log_api_v0.error(LOG_WHERE,
                                 "Could not create nanomsg socket: %s",
                                 nn_strerror(errno));
                return;// 0;
            }

            addr = config_api_v0.get_string(_G.cv_push_addr);

            log_api_v0.debug(LOG_WHERE, "Push address: %s", addr);

            if (nn_connect(socket, addr) < 0) {
                log_api_v0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                                 addr,
                                 nn_strerror(errno));
                return;// 0;
            }
            _G.push_socket = socket;
        }


////
        socket = nn_socket(AF_SP, NN_PUB);
        if (socket < 0) {
            log_api_v0.error(LOG_WHERE, "Could not create nanomsg socket: %s",
                             nn_strerror(errno));
            return;// 0;
        }

        addr = config_api_v0.get_string(_G.cv_log_addr);

        log_api_v0.debug(LOG_WHERE, "LOG address: %s", addr);

        if (nn_bind(socket, addr) < 0) {
            log_api_v0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                             addr,
                             nn_strerror(errno));
            return;// 0;
        }
        _G.log_socket = socket;

        log_api_v0.register_handler(nano_log_handler, &_G.log_socket);

        consoleserver::register_command("console_server.ready", cmd_ready);
    }



    static void _shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        nn_close(_G.push_socket);
        //nn_close(_G.log_socket);
        nn_close(_G.rpc_socket);
    }

    static void _update() {
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


    extern "C" void *consoleserver_get_module_api(int api) {
        switch (api) {
            case PLUGIN_EXPORT_API_ID: {
                static struct module_export_api_v0 module = {0};

                module.init = _init;
                module.init_api = _init_api;
                module.shutdown = _shutdown;
                module.init_cvar = _init_cvar;
                module.update = _update;

                return &module;
            }

            default:
                return NULL;
        }
    }

}
