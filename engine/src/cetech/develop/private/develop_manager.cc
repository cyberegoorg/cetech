#include "celib/memory/memory.h"
#include "celib/container/hash.inl.h"
#include "celib/container/eventstream.inl.h"

#include "cetech/develop/develop_manager.h"
#include "cetech/develop/console_server.h"
#include "cetech/application/application.h"
#include "cetech/os/os.h"

#include "rapidjson/document.h"

namespace cetech {
    namespace {
        using namespace develop_manager;

        typedef void (* to_json_fce_t)(const void*, rapidjson::Document&);

        static thread_local char _stream_buffer[64 * 1024];
        static thread_local uint32_t _stream_buffer_count;

        struct DevelopManagerData {
            EventStream stream;
            Hash < to_json_fce_t > to_json;
            Hash < const char* > type_to_string;

            DevelopManagerData(Allocator & allocator) : stream(allocator), to_json(allocator),
                                                        type_to_string(allocator) {}
        };

        struct Globals {
            static const int MEMORY = sizeof(DevelopManagerData);
            char buffer[MEMORY];

            DevelopManagerData* data;

            Globals() : data(0) {}
        } _globals;


        void flush_stream_buffer() {
            eventstream::write(develop_manager::event_stream(), _stream_buffer, _stream_buffer_count);
            _stream_buffer_count = 0;
        }

        template < typename T >
        void push(uint32_t type, const T& event) {
            const uint32_t sz = sizeof(eventstream::Header) + sizeof(T);

            if ((_stream_buffer_count + sz) >= 64 * 1024) {
                flush_stream_buffer();
            }

            char* p = _stream_buffer + _stream_buffer_count;

            eventstream::Header* h = (eventstream::Header*)p;
            h->type = type;
            h->size = sizeof(T);

            _stream_buffer_count += sz;

            *(T*)(p + sizeof(eventstream::Header)) = event;
        }
    }

    namespace develop_manager {
        void clear() {
            eventstream::clear(_globals.data->stream);
        }

        EventStream& event_stream() {
            return _globals.data->stream;
        }

        void register_type(EventType type, const char* type_str, to_json_fce_t fce) {
            hash::set(_globals.data->to_json, type, fce);
            hash::set(_globals.data->type_to_string, type, type_str);
        }

        void send_buffer() {
            flush_stream_buffer();

            if (!console_server::has_clients() || eventstream::empty(_globals.data->stream)) {
                return;
            }

            eventstream::event_it it = 0;
            while (eventstream::valid(_globals.data->stream, it)) {

                eventstream::Header* header = eventstream::header(_globals.data->stream, it);

                const char* type_str = hash::get < const char* >
                                       (_globals.data->type_to_string,
                                        (EventType)header->type,
                                        "NONE");

                rapidjson::Document json_data;
                json_data.SetObject();

                json_data.AddMember("type", "debug_event", json_data.GetAllocator());
                json_data.AddMember("etype", rapidjson::Value(type_str, strlen(type_str)), json_data.GetAllocator());

                to_json_fce_t to_json_fce = hash::get < to_json_fce_t >
                                            (_globals.data->to_json, header->type, nullptr);
                if (to_json_fce) {
                    to_json_fce(eventstream::event < void* > (_globals.data->stream, it), json_data);
                }

                console_server::send_json_document(json_data);

                it = eventstream::next(_globals.data->stream, it);
            }
        }

        void push_begin_frame() {
            BeginFrameEvent event = {
                .get_frame_id = application_globals::app().get_frame_id(),
                .time = os::get_ticks()
            };

            push(EVENT_BEGIN_FRAME, event);
        }

        void push_end_frame() {
            EndFrameEvent event = {
                .get_frame_id = application_globals::app().get_frame_id(),
                .time = os::get_ticks()
            };

            push(EVENT_END_FRAME, event);
        }

        void push_record_float(const char* name, const float value) {
            RecordFloatEvent event = {
                .name = strdup(name),
                .get_frame_id = application_globals::app().get_frame_id(),
                .value = value
            };
            push(EVENT_RECORD_FLOAT, event);
        }



        static void beginframe_to_json(const void* event, rapidjson::Document& document) {
            BeginFrameEvent* e = (BeginFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        static void endframe_to_json(const void* event, rapidjson::Document& document) {
            EndFrameEvent* e = (EndFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        static void recordfloat_to_json(const void* event, rapidjson::Document& document) {
            RecordFloatEvent* e = (RecordFloatEvent*)event;

            document.AddMember("name", rapidjson::Value(e->name, strlen(e->name)), document.GetAllocator());
            document.AddMember("value", e->value, document.GetAllocator());
        }


    }

    namespace develop_manager_globals {
        void init() {
            char* p = _globals.buffer;
            _globals.data = new(p) DevelopManagerData(memory_globals::default_allocator());

            register_type(EVENT_BEGIN_FRAME, "EVENT_BEGIN_FRAME", beginframe_to_json);
            register_type(EVENT_END_FRAME, "EVENT_END_FRAME", endframe_to_json);
            register_type(EVENT_RECORD_FLOAT, "EVENT_RECORD_FLOAT", recordfloat_to_json);
        }

        void shutdown() {
            _globals = Globals();
        }
    }
}
