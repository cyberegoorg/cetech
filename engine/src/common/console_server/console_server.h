#pragma once

#include "common/memory/memory_types.h"

#include "rapidjson/document.h"

namespace cetech {
    class ConsoleServer {
    public:
        typedef void (* command_clb_t)(const rapidjson::Document&, rapidjson::Document&);

        virtual ~ConsoleServer(){};

        /*! Execute console server operation.
         */
        virtual void tick() = 0;

        /*! Register command.
         * \param name Name
         * \param clb Callback.
         */
        virtual void register_command(const char* name, const command_clb_t clb) = 0;

        /*! Has console server any client?
         * \return True if has else return false./
         */
        virtual bool has_clients() = 0;

        /*! Send JSON document.
         * \param document Json document.
         */
        virtual void send_json_document(const rapidjson::Document& document) = 0;
        
        static ConsoleServer* make(Allocator& alocator);
        static void destroy(Allocator& alocator, ConsoleServer* cs);
    };
}