#pragma once

#include "common/resource/resource_manager.h"

#include <inttypes.h>

namespace cetech {
    class Device {
        public:
            virtual ~Device() {};
            
            virtual uint32_t get_frame_id() = 0;
            virtual float get_delta_time() = 0;

            virtual void init(int argc, const char** argv) = 0;
            virtual void shutdown() = 0;
            virtual void run() = 0;
            
            virtual ResourceManager* resource_manager() = 0;
    };

    namespace device_globals {
        void init();
        void shutdown();
        
        Device& device();
    }
}