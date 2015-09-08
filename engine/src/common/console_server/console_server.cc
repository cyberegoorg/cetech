#include "common/console_server/console_server.h"

#include "common/memory/memory.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/container/queue.h"
#include "common/string/stringid.h"
#include "cvars/cvars.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "enet/enet.h"

namespace cetech {
    namespace console_server_globals {
        struct ConsoleServer {
            ENetAddress server_addr;
            ENetHost* server_host;

            Array < ENetPeer > client_peer;
            Queue < int > peer_free_queue;

            Hash < command_clb_t > cmds;

            ConsoleServer(Allocator & allocator) : client_peer(allocator), peer_free_queue(allocator),
                                                   cmds(allocator) {}
        };

        ConsoleServer* _cs;
    }

    namespace console_server_globals {
        void register_command(const char* name, const command_clb_t clb) {
            hash::set(_cs->cmds, stringid64::from_cstring(name), clb);
        }

        void init() {
            _cs = MAKE_NEW(memory_globals::default_allocator(), ConsoleServer, memory_globals::default_allocator());

            _cs->server_addr.host = ENET_HOST_ANY;
            _cs->server_addr.port = cvars::console_server_port.value_i;
            _cs->server_host = enet_host_create(&_cs->server_addr, 32, 10, 0, 0);
        }

        void shutdown() {
            enet_host_destroy( _cs->server_host);

            MAKE_DELETE(memory_globals::default_allocator(), ConsoleServer, _cs);
            _cs = nullptr;
        }


        bool validate_packet(rapidjson::Document& document, const char* packet, const uint32_t size) {
            document.Parse(packet);

            /* Name */
            if (document.HasParseError()) {
                log::error("console_server", "Packet parse error: %s", GetParseError_En(
                               document.GetParseError()), document.GetErrorOffset());
                return false;
            }

            if (!document.HasMember("name")) {
                log::error("console_server", "Packet require key \"name\"");
                return false;
            }

            return true;
        }

        void parse_packet(uint32_t client, const char* packet, const uint32_t size) {
            log::debug("client_server", "Client msg: %s", packet);

            rapidjson::Document document;
            if (!validate_packet(document, packet, size)) {
                return;
            }

            // Find command
            command_clb_t cmd = hash::get < command_clb_t >
                                (_cs->cmds, stringid64::from_cstring(document["name"].GetString()), nullptr);
            if (cmd == nullptr) {
                log::error("console_server", "Command \"%s\" not found.", document["name"].GetString());
                return;
            }

            rapidjson::Document document_out;
            cmd(document, document_out);

            if (document_out.IsObject()) {}
        }

        void tick() {
            ENetEvent Event;

            while (enet_host_service(_cs->server_host, &Event, 0) > 0) {

                switch (Event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    log::info("console_server", "Client connected.");
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    log::info("console_server", "Client disconnected.");
                    break;

                case ENET_EVENT_TYPE_RECEIVE: {
                    char buff[4096] = {0};
                    strncpy(buff, (char*)Event.packet->data, Event.packet->dataLength);
                    parse_packet(0, buff, Event.packet->dataLength);
                    enet_packet_destroy(Event.packet);
                    break;
                }

                default:
                    break;
                }
            }
        }
    }
}