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
            develop_events::BeginFrameEvent* e = (develop_events::BeginFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        void endframe_to_json(const char* event, rapidjson::Document& document) {
            develop_events::EndFrameEvent* e = (develop_events::EndFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }

        void recordfloat_to_json(const char* event, rapidjson::Document& document) {
            develop_events::RecordFloatEvent* e = (develop_events::RecordFloatEvent*)event;

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

            event_it event = eventstream::first(_de->stream);
            while (event != eventstream::end(_de->stream)) {
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
                    to_json_fce(eventstream::data < char > (_de->stream, event), json_data);
                }

                console_server_globals::send_json_document(json_data);
                event = eventstream::next(_de->stream, event);
            }
        }

        void add_begin_frame() {
            develop_events::BeginFrameEvent* e = eventstream::prepare_new < develop_events::BeginFrameEvent >
                                                 (_de->stream, develop_events::EVENT_BEGIN_FRAME);
            e->time = runtime::get_ticks();
        }

        void add_end_frame() {
            develop_events::EndFrameEvent* e = eventstream::prepare_new < develop_events::EndFrameEvent >
                                               (_de->stream, develop_events::EVENT_END_FRAME);
            e->time = runtime::get_ticks();
        }

        void add_record_float(const char* name, const float value) {
            develop_events::RecordFloatEvent* e = eventstream::prepare_new < develop_events::RecordFloatEvent >
                                                  (_de->stream, develop_events::EVENT_RECORD_FLOAT);

            e->name = strdup(name);
            e->value = value;
        }
    }

};
