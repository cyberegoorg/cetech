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

    void console_server_handler(const LogLevel::Enum level,
                                const time_t time,
                                const char* where,
                                const char* msg,
                                void* data) {

        static const char* level_to_str[] = { "I", "W", "E", "D" };

        CE_UNUSED(data);

        ConsoleServer& cs = application_globals::app().console_server();

        if (!cs.has_clients()) {
            return;
        }

        rapidjson::Document json_data;
        json_data.SetObject();

        json_data.AddMember("type", "log", json_data.GetAllocator());

        json_data.AddMember("time", rapidjson::Value((int64_t)time), json_data.GetAllocator());
        json_data.AddMember("level", rapidjson::Value(level_to_str[level], 1), json_data.GetAllocator());
        json_data.AddMember("where", rapidjson::Value(where, strlen(where)), json_data.GetAllocator());
        json_data.AddMember("msg", rapidjson::Value(msg, strlen(msg)), json_data.GetAllocator());

        cs.send_json_document(json_data);
    }

    class ConsoleServerImplementation : public ConsoleServer {
        friend class ConsoleServer;

        ENetAddress server_addr;
        ENetHost* server_host;

        Array < ENetPeer * > client_peer;
        Queue < int > peer_free_queue;

        Hash < command_clb_t > cmds;

        ConsoleServerImplementation(Allocator & allocator) : client_peer(allocator), peer_free_queue(allocator),
                                                             cmds(allocator) {
            server_addr.host = ENET_HOST_ANY;
            server_addr.port = cvars::console_server_port.value_i;
            server_host = enet_host_create(&server_addr, 32, 10, 0, 0);

            log_globals::log().register_handler(&console_server_handler);
        }

        virtual ~ConsoleServerImplementation() final {
            enet_host_destroy(server_host);
            log_globals::log().unregister_handler(&console_server_handler);
        }

        virtual void register_command(const char* name, const command_clb_t clb) final {
            hash::set(this->cmds, stringid64::from_cstring(name), clb);
        }

        virtual bool has_clients() final {
            return array::size(this->client_peer) != 0;
        }

        virtual void send_json_document(const rapidjson::Document& document) final {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer < rapidjson::StringBuffer > writer(buffer);
            document.Accept(writer);

            ENetPacket* p = enet_packet_create(buffer.GetString(), buffer.GetSize(), ENET_PACKET_FLAG_RELIABLE);
            enet_host_broadcast(server_host, 0, p);
        }

        virtual void tick() final {
            ENetEvent Event;

            while (enet_host_service(this->server_host, &Event, 0) > 0) {

                switch (Event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    int64_t cid = 0;

                    if (queue::size(this->peer_free_queue) > 0) {
                        cid = this->peer_free_queue[0] + 1;
                        queue::pop_front(this->peer_free_queue);
                    } else {
                        array::push_back(this->client_peer, Event.peer);
                        cid = array::size(this->client_peer);
                    }

                    Event.peer->data = (void*)cid;
                    log_globals::log().info("console_server", "Client connected. %d", cid);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    int64_t cid = (int64_t)(Event.peer->data);

                    this->client_peer[cid - 1] = nullptr;
                    queue::push_back(this->peer_free_queue, (int)(cid - 1));

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

            rapidjson::Document document;
            if (!validate_packet(document, packet, size)) {
                return;
            }

            // Find command
            command_clb_t cmd = hash::get < command_clb_t >
                                (this->cmds, stringid64::from_cstring(document["name"].GetString()), nullptr);
            if (cmd == nullptr) {
                log_globals::log().error("console_server", "Command \"%s\" not found.", document["name"].GetString());
                return;
            }

            rapidjson::Document document_out;
            cmd(document, document_out);

            if (document_out.IsObject()) {}
        }
    };

    ConsoleServer* ConsoleServer::make(Allocator& allocator) {
        return MAKE_NEW(allocator, ConsoleServerImplementation, allocator);
    }

    void ConsoleServer::destroy(Allocator& allocator, ConsoleServer* cs) {
        MAKE_DELETE(allocator, ConsoleServer, cs);
    }
}