#include "develop_manager.h"

#include "common/memory/memory.h"
#include "common/container/hash.inl.h"
#include "common/container/eventstream.inl.h"
#include "console_server.h"

#include "device.h"
#include "os/os.h"

#include "rapidjson/document.h"

namespace cetech {
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
        uint32_t get_frame_id;
        const char* name;
        float value;
    };

    static thread_local char _stream_buffer[64 * 1024];
    static thread_local uint32_t _stream_buffer_count;

    class DevelopManagerImplementation : public DevelopManager {
        friend class DevelopManager;

        typedef void (* to_json_fce_t)(const void*, rapidjson::Document&);

        EventStream stream;

        Hash < to_json_fce_t > to_json;
        Hash < const char* > type_to_string;


        DevelopManagerImplementation(Allocator & allocator) : stream(allocator), to_json(allocator), type_to_string(
                                                                  allocator) {
            register_type(EVENT_BEGIN_FRAME, "EVENT_BEGIN_FRAME", beginframe_to_json);
            register_type(EVENT_END_FRAME, "EVENT_END_FRAME", endframe_to_json);
            register_type(EVENT_RECORD_FLOAT, "EVENT_RECORD_FLOAT", recordfloat_to_json);
        }

        virtual void clear() final {
            eventstream::clear(stream);
        }

        virtual EventStream& event_stream() final {
            return stream;
        }

        void register_type(EventType type, const char* type_str, to_json_fce_t fce) {
            hash::set(this->to_json, type, fce);
            hash::set(this->type_to_string, type, type_str);
        }

        virtual void send_buffer() final {
            ConsoleServer& cs = device_globals::device().console_server();

            flush_stream_buffer();

            if (!cs.has_clients() || eventstream::empty(this->stream)) {
                return;
            }

            eventstream::event_it it = 0;
            while (eventstream::valid(this->stream, it)) {

                eventstream::Header* header = eventstream::header(this->stream, it);

                const char* type_str = hash::get < const char* >
                                       (this->type_to_string,
                                        (EventType)header->type,
                                        "NONE");

                rapidjson::Document json_data;
                json_data.SetObject();

                json_data.AddMember("type", "debug_event", json_data.GetAllocator());
                json_data.AddMember("etype", rapidjson::Value(type_str, strlen(type_str)), json_data.GetAllocator());

                to_json_fce_t to_json_fce = hash::get < to_json_fce_t >
                                            (this->to_json, header->type, nullptr);
                if (to_json_fce) {
                    to_json_fce(eventstream::event < void* > (this->stream, it), json_data);
                }

                cs.send_json_document(json_data);

                it = eventstream::next(this->stream, it);
            }
        }

        virtual void push_begin_frame() final {
            BeginFrameEvent event = {
                .get_frame_id = device_globals::device().get_frame_id(),
                .time = os::get_ticks()
            };

            push(EVENT_BEGIN_FRAME, event);
        }

        virtual void push_end_frame() final {
            EndFrameEvent event = {
                .get_frame_id = device_globals::device().get_frame_id(),
                .time = os::get_ticks()
            };

            push(EVENT_END_FRAME, event);
        }

        virtual void push_record_float(const char* name, const float value) final {
            RecordFloatEvent event = {
                .get_frame_id = device_globals::device().get_frame_id(),
                .name = strdup(name),
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



        void flush_stream_buffer() {
            eventstream::write(
                device_globals::device().develop_manager().event_stream(), _stream_buffer, _stream_buffer_count);
            _stream_buffer_count = 0;
        }
    };


    DevelopManager* DevelopManager::make(Allocator& alocator) {
        return MAKE_NEW(alocator, DevelopManagerImplementation, alocator);
    }

    void DevelopManager::destroy(Allocator& alocator, DevelopManager* dm) {
        MAKE_DELETE(memory_globals::default_allocator(), DevelopManager, dm);
    }
}
