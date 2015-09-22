#pragma once

#include "common/memory/memory.h"
#include "common/container/hash.h"
#include "common/console_server/console_server.h"
#include "common/container/eventstream.h"
#include "common/device.h"
#include "runtime/runtime.h"

#include "rapidjson/document.h"

namespace cetech {
    namespace develop_events {
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

        void beginframe_to_json(const void* event, rapidjson::Document& document) {
            BeginFrameEvent* e = (BeginFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        void endframe_to_json(const void* event, rapidjson::Document& document) {
            EndFrameEvent* e = (EndFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        void recordfloat_to_json(const void* event, rapidjson::Document& document) {
            RecordFloatEvent* e = (RecordFloatEvent*)event;

            document.AddMember("name", rapidjson::Value(e->name, strlen(e->name)), document.GetAllocator());
            document.AddMember("value", e->value, document.GetAllocator());
        }
    }

    namespace develop_globals {
        typedef void (* to_json_fce_t)(const void*, rapidjson::Document&);

        thread_local static char _stream_buffer[64 * 1024] = {0};
        thread_local static uint32_t _stream_buffer_count = 0;

        struct DebugEvents {
            EventStream stream;

            Hash < to_json_fce_t > to_json;
            Hash < const char* > type_to_string;

            DebugEvents(Allocator & allocator) : stream(allocator), to_json(allocator), type_to_string(allocator) {}
        };

        static DebugEvents* _de = nullptr;

        void register_type(develop_events::EventType type, const char* type_str, to_json_fce_t fce) {
            hash::set(_de->to_json, type, fce);
            hash::set(_de->type_to_string, type, type_str);
        }

        void init() {
            _de = MAKE_NEW(memory_globals::default_allocator(), DebugEvents, memory_globals::default_allocator());

            register_type(develop_events::EVENT_BEGIN_FRAME, "EVENT_BEGIN_FRAME", develop_events::beginframe_to_json);
            register_type(develop_events::EVENT_END_FRAME, "EVENT_END_FRAME", develop_events::endframe_to_json);
            register_type(develop_events::EVENT_RECORD_FLOAT, "EVENT_RECORD_FLOAT",
                          develop_events::recordfloat_to_json);
        }

        void shutdown() {
            MAKE_DELETE(memory_globals::default_allocator(), DebugEvents, _de);
        }

        void clear() {
            eventstream::clear(_de->stream);
        }

        void flush_stream_buffer() {
            eventstream::write(_de->stream, _stream_buffer, _stream_buffer_count);
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


        void send_buffer() {
            if (!console_server_globals::has_clients() || eventstream::empty(_de->stream)) {
                return;
            }

            eventstream::event_it it = 0;
            while (eventstream::valid(_de->stream, it)) {

                eventstream::Header* header = eventstream::header(_de->stream, it);

                const char* type_str = hash::get < const char* >
                                       (_de->type_to_string,
                                        (develop_events::EventType)header->type,
                                        "NONE");

                rapidjson::Document json_data;
                json_data.SetObject();

                json_data.AddMember("type", "debug_event", json_data.GetAllocator());
                json_data.AddMember("etype", rapidjson::Value(type_str, strlen(type_str)), json_data.GetAllocator());

                to_json_fce_t to_json_fce = hash::get < to_json_fce_t >
                                            (_de->to_json, header->type, nullptr);
                if (to_json_fce) {
                    to_json_fce(eventstream::event < void* > (_de->stream, it), json_data);
                }

                console_server_globals::send_json_document(json_data);

                it = eventstream::next(_de->stream, it);
            }
        }

        void push_begin_frame() {
            develop_events::BeginFrameEvent event = {
                .get_frame_id = device_globals::device().get_frame_id(),
                .time = runtime::get_ticks()
            };

            push(develop_events::EVENT_BEGIN_FRAME, event);
        }

        void push_end_frame() {
            develop_events::EndFrameEvent event = {
                .get_frame_id = device_globals::device().get_frame_id(),
                .time = runtime::get_ticks()
            };

            push(develop_events::EVENT_END_FRAME, event);
        }

        void push_record_float(const char* name, const float value) {
            develop_events::RecordFloatEvent event = {
                .get_frame_id = device_globals::device().get_frame_id(),
                .name = strdup(name),
                .value = value
            };
            push(develop_events::EVENT_RECORD_FLOAT, event);
        }
    }


};
