#include "common/console_server/console_server.h"

#include "common/memory/memory.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/string/stringid.h"
#include "cvars/cvars.h"


#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

namespace cetech {
    namespace console_server_globals {
        struct ConsoleServer {
            int server_socket;
            int socket_max;
            fd_set socket_set;

            Array < int > client_socket;
            Array < sockaddr_in > client_addr;
            
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

            if ((_cs->server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                log::error("console_server", "Could not create socket");
                return;
            }

            
            const int oldFlag = fcntl(_cs->server_socket, F_GETFL, 0);
            if (fcntl(_cs->server_socket, F_SETFL, oldFlag | O_NONBLOCK) == -1) {
                log::error("console_server", "Could not set nonblocking mode");
                return;
            }
            
            sockaddr_in socket_addr;
            socket_addr.sin_family = AF_INET;
            socket_addr.sin_port = htons(cvars::console_server_port.value_i);
            socket_addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(_cs->server_socket, (sockaddr*)&socket_addr, sizeof(socket_addr)) == -1) {
                log::error("console_server", "Could not bind socket");
                return;
            }

            if (listen(_cs->server_socket, 10) == -1) {
                log::error("console_server", "Could not listen.");
                return;
            }

            _cs->socket_max = _cs->server_socket;
            
            FD_ZERO(&(_cs->socket_set));
            FD_SET(_cs->server_socket, &(_cs->socket_set));
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
            if (select(_cs->socket_max + 1, &(_cs->socket_set), NULL, NULL, NULL) != 0) {
                if (FD_ISSET(_cs->server_socket, &(_cs->socket_set))) {
                    const uint32_t client_id = array::size(_cs->client_socket);

                    sockaddr_in client_ifo;
                    socklen_t addrlen = sizeof(client_ifo);

                    int cl_socket = accept(_cs->server_socket, (sockaddr*)&client_ifo, &addrlen);

                    log::info("console_server", "Client connected.");
                    
                    array::push_back(_cs->client_socket, cl_socket);
                    array::push_back(_cs->client_addr, client_ifo);
                }

                const uint32_t client_len = array::size(_cs->client_socket);
                for (uint32_t i = 0; i < client_len; ++i) {
                    if(_cs->client_socket[i] == 0 ) {
                        continue;
                    }
                    
                    if (FD_ISSET(_cs->client_socket[i], &(_cs->socket_set))) {
                        char buffer[4096] = {0};
                        int lenght = 0;

                        if ((lenght = recv(_cs->client_socket[i], (void*)buffer, 4096 - 1, 0)) <= 0) {
                            close(_cs->client_socket[i]);
                            log::info("console_server", "Client disconnected.");

                            _cs->client_socket[i] = 0;

                        } else {
                            buffer[lenght] = 0;
                            
                            parse_packet(i, buffer, lenght);
                        }
                    }
                }

                FD_ZERO(&(_cs->socket_set));
                FD_SET(_cs->server_socket, &(_cs->socket_set));
                for (uint32_t i = 0; i < client_len; ++i) {
                    if(_cs->client_socket[i] == 0 ) {
                        continue;
                    }
                    
                    if( _cs->client_socket[i] > _cs->socket_max ) {
                         _cs->socket_max = _cs->client_socket[i];
                    }
                    
                    FD_SET(_cs->client_socket[i], &(_cs->socket_set));
                }

                if( _cs->socket_max < _cs->server_socket ) {
                    _cs->socket_max = _cs->server_socket;
                }
                
            }
        }
    }
}