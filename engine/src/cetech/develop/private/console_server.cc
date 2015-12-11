#include "cetech/develop/console_server.h"

#include "celib/macros.h"
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/container/queue.inl.h"
#include "celib/string/stringid.inl.h"
#include "cetech/cvars/cvars.h"

#include "cetech/application/application.h"
#include "cetech/develop/develop_manager.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

#include "enet/enet.h"


#define LOG_FORMAT "---\n"\
                   "level: %s\n"\
                   "where: %s\n"\
                   "time: %ld\n"\
                   "worker: %d\n"\
                   "msg: |\n"\
                   "  %s\n"

namespace cetech {
    namespace {
        using namespace console_server;

        void nanolog_handler(const log::LogLevel::Enum level,
                             const time_t time,
                             const uint32_t worker_id,
                             const char* where,
                             const char* msg,
                             void* data) {

            static const char* level_to_str[] = { "I", "W", "E", "D" };

            int socket = (intptr_t)data;
            
            static __thread char packet[4096];     //!< Final msg.
            int len = snprintf(packet, 4096, LOG_FORMAT, level_to_str[level], where, time, worker_id, msg);
            int bytes = nn_send (socket, packet, len, 0);
            CE_ASSERT(bytes == len);
        }
        
        struct ConsoleServerData {       
            Array < ENetPeer * > client_peer;
            Queue < int > peer_free_queue;
            Hash < command_clb_t > cmds;
            int log_socket;
            int socket;
            
            ConsoleServerData(Allocator & allocator) : client_peer(allocator), peer_free_queue(allocator), cmds(allocator), log_socket(0), socket(0) {}

            ~ConsoleServerData() {
            }
        };

        struct Globals {
            static const int MEMORY = sizeof(ConsoleServerData);
            char buffer[MEMORY];

            ConsoleServerData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;


        bool validate_packet(rapidjson::Document& document,
                             const char* packet,
                             const uint32_t size) {
            CE_UNUSED(size);

            document.Parse(packet);

            if (document.HasParseError()) {
                log::error("console_server", "Packet parse error: %s", GetParseError_En(
                               document.GetParseError()), document.GetErrorOffset());
                return false;
            }

            /* Name */
            if (!document.HasMember("name")) {
                log::error("console_server", "Packet require key \"name\"");
                return false;
            }

            return true;
        }

        void parse_packet(uint32_t client,
                          const char* packet,
                          const uint32_t size) {
            CE_UNUSED(client);

            ConsoleServerData* data = _globals.data;

            rapidjson::Document document;
            if (!validate_packet(document, packet, size)) {
                return;
            }

            // Find command
            command_clb_t cmd = hash::get < command_clb_t >
                                (data->cmds, stringid64::from_cstring(document["name"].GetString()), nullptr);
            if (cmd == nullptr) {
                log::error("console_server", "Command \"%s\" not found.", document["name"].GetString());
                return;
            }

            rapidjson::Document document_out;
            cmd(document, document_out);

            if (document_out.IsObject()) {}
        }
    }

    namespace console_server {
        void init() {
            ConsoleServerData* data = _globals.data;

            int socket = nn_socket (AF_SP, NN_PUB);
            CE_ASSERT(socket >= 0);
            CE_ASSERT(nn_bind (socket, "ws://*:5555") >= 0);          
            data->log_socket = socket;                                   
            log::register_handler(&nanolog_handler, (void*)(intptr_t)socket);
            
            socket = nn_socket (AF_SP, NN_PUB);
            CE_ASSERT(socket >= 0);
            CE_ASSERT(nn_bind (socket, "ws://*:5556") >= 0);
            data->socket = socket;
        }


        void register_command(const char* name,
                              const command_clb_t clb) {
            ConsoleServerData* data = _globals.data;

            hash::set(data->cmds, stringid64::from_cstring(name), clb);
        }

        bool has_clients() {
            ConsoleServerData* data = _globals.data;

            return array::size(data->client_peer) != 0;
        }

        void send_msg(const char* msg, const size_t len) {
            int socket = _globals.data->socket;
            size_t bytes = nn_send (socket, msg, len, 0);
            CE_ASSERT(bytes == len);
        }
        
        void tick() {
//             ConsoleServerData* data = _globals.data;
//             if (!data->server_host) {
//                 return;
//             }
// 
//             ENetEvent Event;
// 
//             auto time = develop_manager::enter_scope("ConsoleServer::tick()");
//             while (enet_host_service(data->server_host, &Event, 0) > 0) {
// 
//                 switch (Event.type) {
//                 case ENET_EVENT_TYPE_CONNECT: {
//                     int64_t cid = 0;
// 
//                     if (queue::size(data->peer_free_queue) > 0) {
//                         cid = data->peer_free_queue[0] + 1;
//                         queue::pop_front(data->peer_free_queue);
//                     } else {
//                         array::push_back(data->client_peer, Event.peer);
//                         cid = array::size(data->client_peer);
//                     }
// 
//                     Event.peer->data = (void*)cid;
//                     log::info("console_server", "Client connected. %d", cid);
//                     break;
//                 }
// 
//                 case ENET_EVENT_TYPE_DISCONNECT: {
//                     int64_t cid = (int64_t)(Event.peer->data);
// 
//                     data->client_peer[cid - 1] = nullptr;
//                     queue::push_back(data->peer_free_queue, (int)(cid - 1));
// 
//                     log::info("console_server", "Client %d disconnected.", cid);
//                     break;
//                 }
// 
// 
//                 case ENET_EVENT_TYPE_RECEIVE: {
//                     char buff[4096] = {0};
//                     strncpy(buff, (char*)Event.packet->data, Event.packet->dataLength);
//                     parse_packet(0, buff, Event.packet->dataLength);
//                     //enet_packet_destroy(Event.packet);
//                     break;
//                 }
// 
//                 default:
//                     break;
//                 }
//             }
// 
//             develop_manager::leave_scope("ConsoleServer::tick()", time);
        }
    }

    namespace console_server_globals {
        void init() {
            log::info("console_server_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) ConsoleServerData(memory_globals::default_allocator());
        }

        void shutdown() {
            log::info("console_server_globals", "Shutdown");
            
            log::unregister_handler(&nanolog_handler);
            
            nn_close(_globals.data->log_socket);
            nn_close(_globals.data->socket);
            
            _globals.data->~ConsoleServerData();
            _globals = Globals();
        }
    }
}
