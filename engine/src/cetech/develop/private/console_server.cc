#include "cetech/develop/console_server.h"

#include "celib/macros.h"
#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"
#include "celib/container/hash.inl.h"
#include "celib/container/queue.inl.h"
#include "celib/string/stringid.inl.h"
#include "cetech/cvars/cvars.h"

#include "cetech/application/application.h"


#include "cetech/log_system/handlers.h"

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "enet/enet.h"

namespace cetech {
    namespace {
        using namespace console_server;

        void console_server_handler(const LogLevel::Enum level,
                                    const time_t time,
                                    const char* where,
                                    const char* msg,
                                    void* data) {

            static const char* level_to_str[] = { "I", "W", "E", "D" };

            CE_UNUSED(data);

            if (!has_clients()) {
                return;
            }

            rapidjson::Document json_data;
            json_data.SetObject();

            json_data.AddMember("type", "log", json_data.GetAllocator());

            json_data.AddMember("time", rapidjson::Value((int64_t)time), json_data.GetAllocator());
            json_data.AddMember("level", rapidjson::Value(level_to_str[level], 1), json_data.GetAllocator());
            json_data.AddMember("where", rapidjson::Value(where, strlen(where)), json_data.GetAllocator());
            json_data.AddMember("msg", rapidjson::Value(msg, strlen(msg)), json_data.GetAllocator());

            send_json_document(json_data);
        }

        struct ConsoleServerData {
            ENetAddress server_addr;
            ENetHost* server_host;

            Array < ENetPeer * > client_peer;
            Queue < int > peer_free_queue;

            Hash < command_clb_t > cmds;

            ConsoleServerData(Allocator & allocator) : client_peer(allocator), peer_free_queue(allocator),
                                                       cmds(allocator) {}

            ~ConsoleServerData() {
                enet_host_destroy(server_host);
                log_globals::log().unregister_handler(&console_server_handler);
            }
        };

        struct Globals {
            static const int MEMORY = sizeof(ConsoleServerData);
            char buffer[MEMORY];

            ConsoleServerData* data;

            Globals() : data(0) {}
        } _globals;


        bool validate_packet(rapidjson::Document& document, const char* packet, const uint32_t size) {
            CE_UNUSED(size);

            document.Parse(packet);

            if (document.HasParseError()) {
                log_globals::log().error("console_server", "Packet parse error: %s", GetParseError_En(
                                             document.GetParseError()), document.GetErrorOffset());
                return false;
            }

            /* Name */
            if (!document.HasMember("name")) {
                log_globals::log().error("console_server", "Packet require key \"name\"");
                return false;
            }

            return true;
        }

        void parse_packet(uint32_t client, const char* packet, const uint32_t size) {
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
                log_globals::log().error("console_server", "Command \"%s\" not found.", document["name"].GetString());
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

            data->server_addr.host = ENET_HOST_ANY;
            data->server_addr.port = cvars::console_server_port.value_i;
            data->server_host = enet_host_create(&data->server_addr, 32, 10, 0, 0);

            log_globals::log().register_handler(&console_server_handler);
        }


        void register_command(const char* name, const command_clb_t clb) {
            ConsoleServerData* data = _globals.data;

            hash::set(data->cmds, stringid64::from_cstring(name), clb);
        }

        bool has_clients() {
            ConsoleServerData* data = _globals.data;

            return array::size(data->client_peer) != 0;
        }

        void send_json_document(const rapidjson::Document& document) {
            ConsoleServerData* data = _globals.data;

            rapidjson::StringBuffer buffer;
            rapidjson::Writer < rapidjson::StringBuffer > writer(buffer);
            document.Accept(writer);

            ENetPacket* p = enet_packet_create(buffer.GetString(), buffer.GetSize(), ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(data->server_host, 0, p);
        }

        void tick() {
            ConsoleServerData* data = _globals.data;

            ENetEvent Event;

            while (enet_host_service(data->server_host, &Event, 0) > 0) {

                switch (Event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    int64_t cid = 0;

                    if (queue::size(data->peer_free_queue) > 0) {
                        cid = data->peer_free_queue[0] + 1;
                        queue::pop_front(data->peer_free_queue);
                    } else {
                        array::push_back(data->client_peer, Event.peer);
                        cid = array::size(data->client_peer);
                    }

                    Event.peer->data = (void*)cid;
                    log_globals::log().info("console_server", "Client connected. %d", cid);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    int64_t cid = (int64_t)(Event.peer->data);

                    data->client_peer[cid - 1] = nullptr;
                    queue::push_back(data->peer_free_queue, (int)(cid - 1));

                    log_globals::log().info("console_server", "Client %d disconnected.", cid);
                    break;
                }


                case ENET_EVENT_TYPE_RECEIVE: {
                    char buff[4096] = {0};
                    strncpy(buff, (char*)Event.packet->data, Event.packet->dataLength);
                    parse_packet(0, buff, Event.packet->dataLength);
                    //enet_packet_destroy(Event.packet);
                    break;
                }

                default:
                    break;
                }
            }
        }
    }

    namespace console_server_globals {
        void init() {
            char* p = _globals.buffer;
            _globals.data = new(p) ConsoleServerData(memory_globals::default_allocator());
        }

        void shutdown() {
            _globals.data->~ConsoleServerData();
            _globals = Globals();
        }
    }
}