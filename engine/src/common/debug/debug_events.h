#pragma once

#include "common/memory/memory.h"
#include "common/container/hash.h"
#include "common/console_server/console_server.h"

#include "rapidjson/document.h"

namespace cetech {
    namespace debug_events {
        enum EventType {
            EVENT_NONE = 0,
            EVENT_BEGIN_FRAME = 1,
            EVENT_END_FRAME = 2,
            EVENT_RECORD_FLOAT = 3,
            EVENT_COUNT
        };

        struct Header {
            uint32_t type;
            uint32_t frame_id;
            uint32_t size;
            char data;
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
            debug_events::BeginFrameEvent *e = (debug_events::BeginFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }
        
        void endframe_to_json(const char* event, rapidjson::Document& document) {
            debug_events::EndFrameEvent *e = (debug_events::EndFrameEvent*)event;
            document.AddMember("time", e->time, document.GetAllocator());
        }
        
        void recordfloat_to_json(const char* event, rapidjson::Document& document) {
            debug_events::RecordFloatEvent *e = (debug_events::RecordFloatEvent*)event;
            
            document.AddMember("name", rapidjson::Value(e->name, strlen(e->name)), document.GetAllocator());
            document.AddMember("value", e->value, document.GetAllocator());
        }
    }

    namespace debug_events_globals {
        typedef void (*to_json_fce_t)(const char*, rapidjson::Document&);

        struct DebugEvents {
            enum {BUFFER_SIZE = 64*1024};

            char events_buffer[BUFFER_SIZE] = {0};
            uint32_t events_buffer_count;

            Hash<to_json_fce_t> to_json;
            Hash<const char*> type_to_string;
            
            DebugEvents(Allocator& allocator): to_json(allocator), type_to_string(allocator){
            }
        };
        
        static DebugEvents* _de = nullptr;

        void register_type(debug_events::EventType type, const char* type_str, to_json_fce_t fce) {
            hash::set(_de->to_json, type, fce);
            hash::set(_de->type_to_string, type, type_str);
        }
        
        void init() {
            _de = MAKE_NEW(memory_globals::default_allocator(), DebugEvents, memory_globals::default_allocator());
            
            register_type(debug_events::EVENT_BEGIN_FRAME, "EVENT_BEGIN_FRAME",debug_events::beginframe_to_json);
            register_type(debug_events::EVENT_END_FRAME, "EVENT_END_FRAME", debug_events::endframe_to_json);
            register_type(debug_events::EVENT_RECORD_FLOAT,"EVENT_RECORD_FLOAT", debug_events::recordfloat_to_json);
        }

        void shutdown() {
            MAKE_DELETE(memory_globals::default_allocator(), DebugEvents, _de);
        }

        void add_events(char* events, uint32_t size) {
            memcpy(_de->events_buffer, events, size);
            _de->events_buffer_count += size;
        };

        void clean_events() {
            memset(_de->events_buffer, 0, DebugEvents::BUFFER_SIZE);
            _de->events_buffer_count = 0;
        }
                
        debug_events::Header* header(char* event) {
            return (debug_events::Header*)event;
        }
        
        char* data(char* event) {
            return &(header(event)->data);
        }

        char* next(char* event) {
            const uint32_t sz = header(event)->size;
            
            if((event + sz + sizeof(debug_events::Header)) >=  (_de->events_buffer + _de->events_buffer_count)) {
                return nullptr;
            }
            
            return event + sz + sizeof(debug_events::Header);
        }

        void send_buffer() {
            if( !console_server_globals::has_clients() || !_de->events_buffer_count) {
                return;
            }

            char* event = _de->events_buffer;
            while(event) {
                const char* type_str = hash::get<const char*>(_de->type_to_string, (debug_events::EventType)header(event)->type, "NONE");
                
                rapidjson::Document json_data;
                json_data.SetObject();
                
                json_data.AddMember("type", "debug_event", json_data.GetAllocator());
                json_data.AddMember("frameid", header(event)->frame_id, json_data.GetAllocator());

                json_data.AddMember("etype", rapidjson::Value(type_str, strlen(type_str)), json_data.GetAllocator());
                
                to_json_fce_t to_json_fce = hash::get<to_json_fce_t>(_de->to_json, header(event)->type, nullptr);
                if(to_json_fce) {
                    to_json_fce(data(event), json_data);
                }
                
                console_server_globals::send_json_document(json_data);
                event = next(event);
            };
        }
    }

    namespace debug_events_local {
        enum {BUFFER_SIZE = 1024};

        thread_local static char events_buffer[BUFFER_SIZE] = {0};
        thread_local static uint32_t events_buffer_count;

        void _clean_events() {
            memset(events_buffer, 0, BUFFER_SIZE);
            events_buffer_count = 0;
        }
        
        void flush_to_global() {
            if(!events_buffer_count) {
                return;
            }

            debug_events_globals::add_events(events_buffer, events_buffer_count);
            _clean_events();
        }


        template<typename T>
        T* add_header(debug_events::EventType type) {
            if( sizeof(debug_events::Header) + sizeof(T) >= BUFFER_SIZE ) {
                flush_to_global();
            }
            
            char* p = events_buffer + events_buffer_count;

            debug_events::Header* h = (debug_events::Header*)p;
            h->type = type;
            h->size = sizeof(T);
            h->frame_id = 1;
            
            events_buffer_count += sizeof(debug_events::Header) + sizeof(T);
            return (T*)(&(h->data));
        }
        
        void add_begin_frame() {
            debug_events::BeginFrameEvent *e = add_header<debug_events::BeginFrameEvent>(debug_events::EVENT_BEGIN_FRAME);
            e->time = runtime::get_ticks();
        }
        
        void add_end_frame() {
            debug_events::EndFrameEvent *e = add_header<debug_events::EndFrameEvent>(debug_events::EVENT_BEGIN_FRAME);
            e->time = runtime::get_ticks();
        }

        void add_record_float(const char* name, const float value) {
            debug_events::RecordFloatEvent *e = add_header<debug_events::RecordFloatEvent>(debug_events::EVENT_RECORD_FLOAT);

            e->name = strdup(name);
            e->value = value;
        }
    }
};