#include "common/console_server/console_server.h"

#include "common/memory/memory.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/string/stringid.h"
#include "cvars/cvars.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "enet/enet.h"

#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace cetech {
    namespace console_server_globals {
        struct ConsoleServer {
            ENetSocket server_socket;
            ENetSocketSet socket_set;

            int socket_max;

            Array < ENetSocket > client_socket;
            Array < ENetAddress > client_addr;
            
            Hash<command_clb_t> cmds;

            ConsoleServer(Allocator& allocator) : client_socket(allocator), client_addr(allocator), cmds(allocator) {}
        };

        ConsoleServer* _cs;
    }

    namespace console_server_globals {
        void register_command(const char* name, const command_clb_t clb) {
            hash::set(_cs->cmds, stringid64::from_cstring(name), clb);
        }

        void init() {
            _cs = MAKE_NEW(memory_globals::default_allocator(), ConsoleServer, memory_globals::default_allocator());

            if ((_cs->server_socket = enet_socket_create(ENET_SOCKET_TYPE_STREAM)) == ENET_SOCKET_NULL) {
                log::error("console_server", "Could not create socket");
                return;
            }


            const int oldFlag = enet_socket_get_option(_cs->server_socket, F_GETFL, 0);
            if (enet_socket_set_option(_cs->server_socket, ENET_SOCKOPT_NONBLOCK, 1)) {
                log::error("console_server", "Could not set nonblocking mode");
                return;
            }

            ENetAddress addr;
            addr.host = ENET_HOST_ANY;
            addr.port = cvars::console_server_port.value_i;
            
            if (enet_socket_bind(_cs->server_socket, &addr) == -1) {
                log::error("console_server", "Could not bind socket");
                return;
            }

            if (enet_socket_listen(_cs->server_socket, 10) == -1) {
                log::error("console_server", "Could not listen.");
                return;
            }

            _cs->socket_max = _cs->server_socket;
            
            ENET_SOCKETSET_EMPTY(_cs->socket_set);
            ENET_SOCKETSET_ADD(_cs->socket_set, _cs->server_socket);
        }

        void shutdown() {
            MAKE_DELETE(memory_globals::default_allocator(), ConsoleServer, _cs);
            _cs = nullptr;
        }

        
        void parse_packet(uint32_t client, const char* packet, const uint32_t size){
            log::info("client_server", "Client msg: %s", packet);
            
            rapidjson::Document document;
            document.Parse(packet);

            if (document.HasParseError()) {
                log::error("console_server", "Packet parse error: %s", GetParseError_En(document.GetParseError()), document.GetErrorOffset());
                return;
            }
            
            if( !document.HasMember("name") ) {
                log::error("console_server", "packer require key \"name\"");
                return;
            }
            
            command_clb_t cmd = hash::get<command_clb_t>(_cs->cmds, stringid64::from_cstring(document["name"].GetString()), nullptr);
            if( cmd == nullptr ) {
                log::error("console_server", "Command \"%s\" not found.", document["name"].GetString());
                return;
            }
            
            rapidjson::Document document_out;
            cmd(document, document_out);
            
            if( document_out.IsObject() ) {
            }
        }
        
        void tick() {
            if (enet_socketset_select(_cs->socket_max + 1, &(_cs->socket_set), NULL, 0) != 0) {
                if (ENET_SOCKETSET_CHECK(_cs->socket_set, _cs->server_socket)) {
                    const uint32_t client_id = array::size(_cs->client_socket);
                    
                    ENetAddress addr;
                    ENetSocket cl_socket = enet_socket_accept(_cs->server_socket, &addr);

                    log::info("console_server", "Client connected.");
                    
                    array::push_back(_cs->client_socket, cl_socket);
                    array::push_back(_cs->client_addr, addr);
                }

                const uint32_t client_len = array::size(_cs->client_socket);
                for (uint32_t i = 0; i < client_len; ++i) {
                    if(_cs->client_socket[i] == 0 ) {
                        continue;
                    }
                    
                    if (ENET_SOCKETSET_CHECK(_cs->socket_set, _cs->client_socket[i])) {
                        char buffer[4096] = {0};
                        
                        ENetBuffer ebuffer;
                        ebuffer.data = buffer;
                        ebuffer.dataLength = 4096; 

                        int lenght = 0;

                        if ((lenght = enet_socket_receive(_cs->client_socket[i], (void*)buffer, 4096 - 1, 0)) <= 0) {
                            close(_cs->client_socket[i]);
                            log::info("console_server", "Client disconnected.");

                            _cs->client_socket[i] = 0;

                        } else {
                            buffer[lenght] = 0;
                            
                            parse_packet(i, buffer, lenght);
                        }
                    }
                }

                ENET_SOCKETSET_EMPTY(_cs->socket_set);
                ENET_SOCKETSET_ADD(_cs->socket_set, _cs->server_socket);
                for (uint32_t i = 0; i < client_len; ++i) {
                    if(_cs->client_socket[i] == 0 ) {
                        continue;
                    }
                    
                    if( _cs->client_socket[i] > _cs->socket_max ) {
                         _cs->socket_max = _cs->client_socket[i];
                    }
                    
                    ENET_SOCKETSET_ADD(_cs->socket_set, _cs->client_socket[i]);
                }

                if( _cs->socket_max < _cs->server_socket ) {
                    _cs->socket_max = _cs->server_socket;
                }
                
            }
        }
    }
}