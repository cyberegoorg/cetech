#pragma once

#include "celib/memory/memory.h"
#include "celib/container/container_types.h"
#include "rapidjson/document.h"

namespace cetech {
    namespace develop_manager {
        enum EventType {
            EVENT_NONE = 0,

            EVENT_BEGIN_FRAME = 1,
            EVENT_END_FRAME = 2,

            EVENT_RECORD_FLOAT = 3,

            EVENT_MEMORY_ALLOC = 4,
            EVENT_MEMORY_FREE = 5,

            EVENT_COUNT
        };

        struct BeginFrameEvent {
            uint32_t get_frame_id;
            uint32_t time;
        };

        struct EndFrameEvent {
            uint32_t get_frame_id;
            uint32_t time;
        };

        struct RecordFloatEvent {
            const char* name;
            uint32_t get_frame_id;
            float value;
        };

        void clear();
        void send_buffer();

        EventStream& event_stream();

        void push_begin_frame();
        void push_end_frame();
        void push_record_float(const char* name, const float value);
    }

    namespace develop_manager_globals {
        void init();
        void shutdown();
    }
}
