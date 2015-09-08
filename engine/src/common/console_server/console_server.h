#pragma once

#include "rapidjson/document.h"

namespace cetech {
    namespace console_server_globals {
        typedef void (* command_clb_t)(const rapidjson::Document&, rapidjson::Document&);

        void register_command(const char* name, const command_clb_t clb);

        bool has_clients();
        
        void send_message(const char *type, rapidjson::Value& data);

        void init();
        void shutdown();
        void tick();
    }
}