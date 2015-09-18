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

        void frame_start();
        void frame_end();

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

        /*! Add frame event.
         * \param type Message type.
         * \param data Message data.
         */
        void add_frame_event(const char* type, rapidjson::Value& data);
    }
}