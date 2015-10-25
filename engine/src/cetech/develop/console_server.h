#pragma once

#include "celib/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    namespace console_server {
        typedef void (* command_clb_t)(const rapidjson::Document&, rapidjson::Document&);

        void init();

        /*! Execute console server operation.
         */
        void tick();

        /*! Register command.
         * \param name Name
         * \param clb Callback.
         */
        void register_command(const char* name, const command_clb_t clb);

        /*! Has console server any client?
         * \return True if has else return false./
         */
        bool has_clients();

        /*! Send JSON document.
         * \param document Json document.
         */
        void send_json_document(const rapidjson::Document& document);
    }

    namespace console_server_globals {
        void init();
        void shutdown();
    }
}