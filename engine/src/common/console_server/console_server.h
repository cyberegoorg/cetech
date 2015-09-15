#pragma once

#include "rapidjson/document.h"

namespace cetech {
    namespace console_server_globals {
        typedef void (* command_clb_t)(const rapidjson::Document&, rapidjson::Document&);
        
        /*! Init.
         */
        void init();
        
        /*! Shutdown.
         */
        void shutdown();

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
        
        /*! Send message.
         * \param type Message type.
         * \param data Message data.
         */
        void send_message(const char* type, rapidjson::Value& data);
    }
}