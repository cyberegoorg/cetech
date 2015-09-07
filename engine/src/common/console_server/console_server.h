#pragma once

#include "rapidjson/document.h"

namespace cetech {
    namespace console_server_globals {
        typedef void (* command_clb_t)(const rapidjson::Document&, rapidjson::Document&);

        void register_command(const char* name, const command_clb_t clb);

        void init();
        void shutdown();
        void tick();
    }
}