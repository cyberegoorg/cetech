#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "rapidjson/document.h"

#include "celib/time.h"
#include "celib/memory/memory.h"
#include "celib/container/types.h"

#include "cetech/task_manager/task_manager.h"

/*******************************************************************************
**** Interface
*******************************************************************************/
namespace cetech {

    /***************************************************************************
    **** Develop manager.
    ***************************************************************************/
    namespace develop_manager {

        /***********************************************************************
        **** Event type enum.
        ***********************************************************************/
        enum EventType {
            EVENT_NONE = 0,

            EVENT_BEGIN_FRAME = 1,
            EVENT_END_FRAME = 2,
            EVENT_RECORD_FLOAT = 3,
            EVENT_SCOPE = 6,

            EVENT_COUNT
        };

        /***********************************************************************
        **** Constatnts enum.
        ***********************************************************************/
        enum {
            EVENT_NAME_MAX_LEN = 32
        };

        /***********************************************************************
        **** Begin frame event.
        ***********************************************************************/
        struct BeginFrameEvent {
            uint32_t frame_id;
            time::PerfTimer time;
        };

        void push_begin_frame();

        /***********************************************************************
        **** End frame event.
        ***********************************************************************/
        struct EndFrameEvent {
            uint32_t frame_id;
            time::PerfTimer time;
        };

        void push_end_frame();

        /***********************************************************************
        **** Profile scope evet.
        ***********************************************************************/
        struct ScopeEvent {
            time::PerfTimer start;
            time::PerfTimer end;
            const char* name;
            uint32_t frame_id;
            uint32_t depth;
            uint32_t worker_id;
        };

        time::PerfTimer enter_scope(const char* name);
        void leave_scope(const char* name,
                         time::PerfTimer& start_time);

        /***********************************************************************
        **** Record float evet.
        ***********************************************************************/
        struct RecordFloatEvent {
            uint32_t frame_id;
            float value;
            char name[EVENT_NAME_MAX_LEN];
        };

        void push_record_float(const char* name,
                               const float value);


        /***********************************************************************
        **** Clear develop manager events.
        ***********************************************************************/
        void clear();

        /***********************************************************************
        **** Send develop manager event buffer.
        ***********************************************************************/
        void send_buffer();

        /***********************************************************************
        **** Get event stream.
        ***********************************************************************/
        EventStream& event_stream();

        /***********************************************************************
        **** Flush stream buffer to event stream.
        ***********************************************************************/
        void flush_stream_buffer();

        /***********************************************************************
        **** Flush stream buffers for all workers.
        ***********************************************************************/
        void flush_all_stream_buffer();
    }

    /***************************************************************************
    **** Develop manager globals function.
    ***************************************************************************/
    namespace develop_manager_globals {

        /***********************************************************************
        **** Init system.
        ***********************************************************************/
        void init();

        /***********************************************************************
        **** Shutdown system.
        ***********************************************************************/
        void shutdown();
    }
}