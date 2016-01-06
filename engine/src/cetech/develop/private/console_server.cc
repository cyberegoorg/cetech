#include "cetech/develop/console_server.h"

#include <cerrno>

#include "celib/macros.h"
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/container/queue.inl.h"
#include "celib/string/stringid.inl.h"
#include "cetech/cvars/cvars.h"

#include "cetech/application/application.h"
#include "cetech/develop/develop_manager.h"

#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"
#include "nanomsg/pipeline.h"
#include "nanomsg/reqrep.h"

#define LOG_FORMAT "#log\n" \
    "level: %s\n" \
    "where: %s\n" \
    "time: %ld\n" \
    "worker: %d\n" \
    "msg: |\n" \
    "  %s\n"

static const char* level_to_str[] = { "I", "W", "E", "D" };

namespace cetech {
    namespace {
        using namespace console_server;

        void nanolog_handler(const log::LogLevel::Enum level,
                             const time_t time,
                             const uint32_t worker_id,
                             const char* where,
                             const char* msg,
                             void* data) {

            int socket = (intptr_t)data;

            char packet[4096];     //!< Final msg.
            int len = snprintf(packet, 4096, LOG_FORMAT, level_to_str[level], where, time, worker_id, msg);
            int bytes = nn_send(socket, packet, len, 0);
            CE_ASSERT("console_server", bytes == len);
        }

        struct ConsoleServerData {
            Hash < command_clb_t > cmds;
            int dev_pub_socket;
            int dev_rep_socket;

            ConsoleServerData(Allocator & allocator) : cmds(allocator), dev_pub_socket(0), dev_rep_socket(0) {}

            ~ConsoleServerData() {}
        };

        struct Globals {
            static const int MEMORY = sizeof(ConsoleServerData);
            char buffer[MEMORY];

            ConsoleServerData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;


        void parse_packet(uint32_t client,
                          const char* packet,
                          const uint32_t size) {
            CE_UNUSED(client);

            ConsoleServerData* data = _globals.data;

            mpack_tree_t tree;
            mpack_tree_init(&tree, packet, size);
            mpack_node_t root = mpack_tree_root(&tree);

            // Find command
            char cmd_name[256];
            mpack_node_copy_cstr(mpack_node_map_cstr(root, "name"), cmd_name, 256);
                       
            command_clb_t cmd = hash::get < command_clb_t > (data->cmds, stringid64::from_cstring(cmd_name), nullptr);
            if (cmd == nullptr) {
                log::error("console_server", "Command \"%s\" not found.", cmd_name);
                return;
            }
            
            cmd(root);
        }
    }

    namespace console_server {
        void register_command(const char* name,
                              const command_clb_t clb) {
            ConsoleServerData* data = _globals.data;

            hash::set(data->cmds, stringid64::from_cstring(name), clb);
        }

        void send_msg(const char* buffer,
                      size_t size) {
            int socket = _globals.data->dev_pub_socket;
            size_t bytes = nn_send(socket, buffer, size, 0);
            CE_ASSERT("console_server", bytes == size);
        }

        void send_msg(const Array < char >& msg) {
            send_msg(array::begin(msg), array::size(msg));
        }

        void tick() {
            int socket = _globals.data->dev_rep_socket;

            auto time = develop_manager::enter_scope("ConsoleServer::tick()");

            char* buf = NULL;
            int bytes = nn_recv(socket, &buf, NN_MSG, NN_DONTWAIT);
            if (bytes < 0) {
                CE_ASSERT("console_server", errno == EAGAIN );
                goto end;
            }

            //log::debug("ddddd", "parse");
            parse_packet(0, buf, bytes);
            nn_freemsg(buf);
            
//             char* data;
//             size_t size;
//             mpack_writer_t writer;
//             mpack_writer_init_growable(&writer, &data, &size);
// 
//             mpack_start_map(&writer, 1);
//             mpack_write_cstr(&writer, "status");
//             mpack_write_i32(&writer, 200);
//             mpack_finish_map(&writer);
//             CE_ASSERT("develop_manager", mpack_writer_destroy(&writer) == mpack_ok);
//             
//             bytes = nn_send(socket, data, size, 0);
            //CE_ASSERT("console_server", bytes == size);
end:
            develop_manager::leave_scope("ConsoleServer::tick()", time);
        }
    }

    namespace console_server_globals {
        void init() {
            log::info("console_server_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) ConsoleServerData(memory_globals::default_allocator());

            int socket = nn_socket(AF_SP, NN_PUB);
            CE_ASSERT("console_server", socket >= 0);
            CE_ASSERT("console_server", nn_bind(socket, "ws://*:5556") >= 0);
            _globals.data->dev_pub_socket = socket;
            log::register_handler(&nanolog_handler, (void*)(intptr_t)socket);

            socket = nn_socket(AF_SP, NN_PULL);
            CE_ASSERT("console_server", socket >= 0);
            CE_ASSERT("console_server", nn_bind(socket, "ws://*:5557") >= 0);
            _globals.data->dev_rep_socket = socket;
        }

        void shutdown() {
            log::info("console_server_globals", "Shutdown");

            log::unregister_handler(&nanolog_handler);

            nn_close(_globals.data->dev_rep_socket);
            nn_close(_globals.data->dev_pub_socket);

            _globals.data->~ConsoleServerData();
            _globals = Globals();
        }
    }
}
