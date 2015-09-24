#include "console_server.h"

#include "common/memory/memory.h"
#include "common/container/array.inl.h"
#include "common/container/hash.inl.h"
#include "common/container/queue.inl.h"
#include "common/string/stringid.inl.h"
#include "cvars/cvars.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "enet/enet.h"

namespace cetech {
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
        }

        virtual ~ConsoleServerImplementation() final {
            enet_host_destroy(server_host);
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
                    log::info("console_server", "Client connected. %d", cid);
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    int64_t cid = (int64_t)(Event.peer->data);

                    this->client_peer[cid - 1] = nullptr;
                    queue::push_back(this->peer_free_queue, (int)(cid - 1));

                    log::info("console_server", "Client %d disconnected.", cid);
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
            //log::debug("client_server", "Client msg: %s", packet);

            rapidjson::Document document;
            if (!validate_packet(document, packet, size)) {
                return;
            }

            // Find command
            command_clb_t cmd = hash::get < command_clb_t >
                                (this->cmds, stringid64::from_cstring(document["name"].GetString()), nullptr);
            if (cmd == nullptr) {
                log::error("console_server", "Command \"%s\" not found.", document["name"].GetString());
                return;
            }

            rapidjson::Document document_out;
            cmd(document, document_out);

            if (document_out.IsObject()) {}
        }
    };

    ConsoleServer* ConsoleServer::make(Allocator& alocator) {
        return MAKE_NEW(alocator, ConsoleServerImplementation, alocator);
    }

    void ConsoleServer::destroy(Allocator& alocator, ConsoleServer* cs) {
        MAKE_DELETE(memory_globals::default_allocator(), ConsoleServer, cs);
    }
}