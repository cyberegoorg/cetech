#pragma once

#include "celib/memory/memory.h"
#include "celib/container/container_types.h"
#include "cetech/task_manager/task_manager.h"
#include "rapidjson/document.h"

#include "celib/time.h"

namespace cetech {
    namespace develop_manager {
        enum EventType {
            EVENT_NONE = 0,

            EVENT_BEGIN_FRAME = 1,
            EVENT_END_FRAME = 2,
            EVENT_RECORD_FLOAT = 3,
            EVENT_SCOPE = 6,

            EVENT_COUNT
        };

        enum {
            EVENT_NAME_MAX_LEN = 32
        };

        struct BeginFrameEvent {
            uint32_t frame_id;
            time::PerfTimer time;
        };

        struct EndFrameEvent {
            uint32_t frame_id;
            time::PerfTimer time;
        };

        struct ScopeEvent {
            time::PerfTimer start;
            time::PerfTimer end;
            const char* name;
            uint32_t frame_id;
            uint32_t depth;
            uint32_t worker_id;
        };

        struct RecordFloatEvent {
            uint32_t frame_id;
            float value;
            char name[EVENT_NAME_MAX_LEN];
        };

        void clear();
        void send_buffer();

        EventStream& event_stream();

        void push_begin_frame();

        void push_end_frame();

        time::PerfTimer enter_scope(const char* name);

        void leave_scope(const char* name,
                         time::PerfTimer& start_time);

        void push_record_float(const char* name,
                               const float value);

        void flush_stream_buffer();

        void flush_all_stream_buffer();
    }

    namespace develop_manager_globals {
        void init();
        void shutdown();
    }
}
