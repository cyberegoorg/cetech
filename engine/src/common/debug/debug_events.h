#pragma once

#include "common/memory/memory.h"
#include "common/container/hash.h"
#include "common/console_server/console_server.h"
#include "common/eventstream/eventstream.h"
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
            uint32_t time;
        };

        struct EndFrameEvent {
            uint32_t time;
        };

        struct RecordFloatEvent {
            const char* name;
            float value;
        };

        void beginframe_to_json(const char* event, rapidjson::Document& document) {
            BeginFrameEvent* e = (BeginFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        void endframe_to_json(const char* event, rapidjson::Document& document) {
            EndFrameEvent* e = (EndFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        void recordfloat_to_json(const char* event, rapidjson::Document& document) {
            RecordFloatEvent* e = (RecordFloatEvent*)event;

            document.AddMember("name", rapidjson::Value(e->name, strlen(e->name)), document.GetAllocator());
            document.AddMember("value", e->value, document.GetAllocator());
        }
    }



    namespace develop_eventstream_globals {
        typedef void (* to_json_fce_t)(const char*, rapidjson::Document&);

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

        void send_buffer() {
            if (!console_server_globals::has_clients() || eventstream::empty(_de->stream)) {
                return;
            }

            for (event_it event = eventstream::first(_de->stream); eventstream::is_valid(_de->stream, event);
                 event = eventstream::next(_de->stream, event)) {
                const char* type_str = hash::get < const char* >
                                       (_de->type_to_string,
                                        (develop_events::EventType)eventstream::header(_de->stream, event)->type,
                                        "NONE");

                rapidjson::Document json_data;
                json_data.SetObject();

                json_data.AddMember("type", "debug_event", json_data.GetAllocator());
                json_data.AddMember("etype", rapidjson::Value(type_str, strlen(type_str)), json_data.GetAllocator());

                to_json_fce_t to_json_fce = hash::get < to_json_fce_t >
                                            (_de->to_json, eventstream::header(_de->stream, event)->type, nullptr);
                if (to_json_fce) {
                    to_json_fce(eventstream::data_ptr < char > (_de->stream, event), json_data);
                }

                console_server_globals::send_json_document(json_data);
            }
        }

        thread_local static char _stream_buffer[64 * 1024] = {0};
        thread_local static uint32_t _stream_buffer_count = 0;

        void flush_stream_buffer(EventStream& stream, char* buffer, uint32_t& buffer_count) {
            eventstream::add_events(stream, buffer, buffer_count);
            buffer_count = 0;
        }


        template < typename T >
        T* prepare_new(EventStream& stream, char* buffer, uint32_t& buffer_count, uint32_t type) {
            const uint32_t sz = sizeof(EventStreamHeader) + sizeof(T);
            if ((buffer_count + sz) >= 64 * 1024) {
                flush_stream_buffer(stream, buffer, buffer_count);
            }

            char* p = buffer + buffer_count;

            EventStreamHeader* h = (EventStreamHeader*)p;
            h->type = type;
            h->size = sizeof(T);

            buffer_count += sz;

            return (T*)(&(h->data));
        }


        void add_begin_frame() {
            develop_events::BeginFrameEvent* e = prepare_new < develop_events::BeginFrameEvent >
                                                 (_de->stream, _stream_buffer, _stream_buffer_count,
                                                  develop_events::EVENT_BEGIN_FRAME);
            e->time = runtime::get_ticks();
        }

        void add_end_frame() {
            develop_events::EndFrameEvent* e = prepare_new < develop_events::EndFrameEvent >
                                               (_de->stream, _stream_buffer, _stream_buffer_count,
                                                develop_events::EVENT_END_FRAME);
            e->time = runtime::get_ticks();
        }

        void add_record_float(const char* name, const float value) {
            develop_events::RecordFloatEvent* e = prepare_new < develop_events::RecordFloatEvent >
                                                  (_de->stream, _stream_buffer, _stream_buffer_count,
                                                   develop_events::EVENT_RECORD_FLOAT);

            e->name = strdup(name); // TODO: LEAK; idea: symbol based,
            e->value = value;
        }

    }


};
