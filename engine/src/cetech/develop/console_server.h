#pragma once

#include "celib/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    namespace console_server {
        typedef void (* command_clb_t)(const rapidjson::Document&,
                                       rapidjson::Document&);

        void init();

        void tick();

        void register_command(const char* name,
                              const command_clb_t clb);

        bool has_clients();

        void send_json_document(const rapidjson::Document& document);
    }
    //
    namespace console_server_globals {
        void init();
        void shutdown();
    }
}
