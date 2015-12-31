#include "celib/macros.h"
#include "celib/types.h"
#include "celib/memory/memory.h"
#include "celib/container/hash.inl.h"
#include "celib/container/eventstream.inl.h"

#include "cetech/develop/develop_manager.h"
#include "cetech/develop/console_server.h"
#include "cetech/application/application.h"

#include "cetech/log/log.h"
#include "cetech/thread/thread.h"

#include "rapidjson/document.h"

#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

namespace cetech {
    namespace {
        using namespace develop_manager;

        typedef void (* to_yaml_fce_t)(const void*,
                                       Array < char >& buffer);

        static thread_local char _stream_buffer[64 * 1024] = {0};
        static thread_local uint32_t _stream_buffer_count = 0;

        static thread_local uint32_t _scope_depth = 0;

        struct DevelopManagerData {
            Hash < to_yaml_fce_t > to_yaml;
            Hash < const char* > type_to_string;
            Array < char > buffer;
            EventStream stream;
            Spinlock lock;

            DevelopManagerData(Allocator & allocator) : to_yaml(allocator),
                                                        type_to_string(allocator),
                                                        buffer(allocator),
                                                        stream(allocator) {
                array::set_capacity(stream.stream, 4096);
                array::set_capacity(buffer, 4096);
            }
            ~DevelopManagerData() {}

        };

        struct Globals {
            static const int MEMORY = sizeof(DevelopManagerData);
            char buffer[MEMORY];

            DevelopManagerData* data;

            Globals() : buffer {
                0
            }, data(0) {}
        } _globals;

        template < typename T >
        void push(uint32_t type,
                  const T& event) {
            //log::info("dm", "push: %d %d\n", type, _stream_buffer_count);
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


        void flush_stream_buffer() {
            thread::spin_lock(_globals.data->lock);

            //printf("flush: %d\n", _stream_buffer_count);
            eventstream::write(develop_manager::event_stream(), _stream_buffer, _stream_buffer_count);
            _stream_buffer_count = 0;

            thread::spin_unlock(_globals.data->lock);
        }

        void register_type(EventType type,
                           const char* type_str,
                           to_yaml_fce_t fce) {
            hash::set(_globals.data->to_yaml, type, fce);
            hash::set(_globals.data->type_to_string, type, type_str);
        }

        void send_buffer() {
            if (eventstream::empty(_globals.data->stream)) {
                return;
            }

            Array < char >& buffer = _globals.data->buffer;
            array::clear(buffer);

            char buf[256];
            array::push(buffer, buf, snprintf(buf, 256, "#develop_manager\nevents:\n"));

            eventstream::event_it it = 0;
            while (eventstream::valid(_globals.data->stream, it)) {
                eventstream::Header* header = eventstream::header(_globals.data->stream, it);

                const char* type_str = hash::get < const char* > (_globals.data->type_to_string,
                                                                  (EventType)header->type,
                                                                  "NONE");

                array::push(buffer, buf, snprintf(buf, 256, "  - etype: %s\n", type_str));

                to_yaml_fce_t to_yaml_fce = hash::get < to_yaml_fce_t > (_globals.data->to_yaml, header->type, nullptr);
                CE_ASSERT("develop_manager", to_yaml_fce);

                to_yaml_fce(eventstream::event < void* > (_globals.data->stream, it), buffer);

                it = eventstream::next(_globals.data->stream, it);
            }

            console_server::send_msg(buffer);
        }

        void push_begin_frame() {
            BeginFrameEvent event = {
                .frame_id = application::get_frame_id(),
                .time = time::get_perftime(),
            };

            push(EVENT_BEGIN_FRAME, event);
        }

        void push_end_frame() {
            EndFrameEvent event = {
                .frame_id = application::get_frame_id(),
                .time = time::get_perftime(),
            };

            push(EVENT_END_FRAME, event);
        }

        time::PerfTimer enter_scope(const char* name) {
            CE_UNUSED(name);

            ++_scope_depth;
            return time::get_perftime();
        }

        void leave_scope(const char* name,
                         time::PerfTimer& start_time) {
            --_scope_depth;
            ScopeEvent event = {
                .name = name,
                .worker_id = task_manager::get_worker_id(),
                .start = start_time,
                .end = time::get_perftime(),
                .depth = _scope_depth,
                .frame_id = application::get_frame_id(),
            };

            push(EVENT_SCOPE, event);
        }


        void push_record_float(const char* name,
                               const float value) {
            RecordFloatEvent event = {
                .frame_id = application::get_frame_id(),
                .value = value
            };

            strncpy(event.name, name, 32);

            push(EVENT_RECORD_FLOAT, event);
        }

        static void flush_develop_manager(void* data) {
            CE_UNUSED(data);

            develop_manager::flush_stream_buffer();
        }

        void flush_all_stream_buffer() {
            flush_stream_buffer();

            task_manager::TaskID flush_develop_manager_task1 = task_manager::add_begin(
                "flush_develop_manager_task1",
                flush_develop_manager, nullptr, task_manager::Priority::High,
                NULL_TASK, NULL_TASK, task_manager::WorkerAffinity::WORKER1
                );

            task_manager::TaskID flush_develop_manager_task2 = task_manager::add_begin(
                "flush_develop_manager_task2",
                flush_develop_manager, nullptr, task_manager::Priority::High,
                flush_develop_manager_task1, NULL_TASK, task_manager::WorkerAffinity::WORKER2
                );

            const task_manager::TaskID task_end[] = {
                flush_develop_manager_task2,
                flush_develop_manager_task1,
            };

            task_manager::add_end(task_end, sizeof(task_end) / sizeof(task_manager::TaskID));
            task_manager::wait(flush_develop_manager_task2);
        }

        static void beginframe_to_json(const void* event,
                                       Array < char >& buffer) {
            BeginFrameEvent* e = (BeginFrameEvent*)event;

            static char buf[4096] = {0};

            static char format[] = "    time: %ld\n"
                                   "    time_ns: %ld\n"
                                   "    frame_id: %d\n";

            size_t len = snprintf(buf, 4096, format, time::get_sec(e->time), time::get_nsec(e->time), e->frame_id);
            array::push(buffer, buf, len);
        }

        static void endframe_to_json(const void* event,
                                     Array < char >& buffer) {
            EndFrameEvent* e = (EndFrameEvent*)event;

            static char buf[4096] = {0};

            static const char format[] = "    time: %ld\n"
                                         "    time_ns: %ld\n"
                                         "    frame_id: %d\n";

            size_t len = snprintf(buf, 4096, format, time::get_sec(e->time), time::get_nsec(e->time), e->frame_id);
            array::push(buffer, buf, len);
        }

        static void begintask_to_json(const void* event,
                                      Array < char >& buffer) {
            ScopeEvent* e = (ScopeEvent*)event;

            static char buf[4096] = {0};
            static const char format[] = "    name: %s\n"
                                         "    frame_id: %d\n"
                                         "    start: %ld\n"
                                         "    start_ns: %ld\n"
                                         "    end: %ld\n"
                                         "    end_ns: %ld\n"
                                         "    worker_id: %d\n"
                                         "    depth: %d\n";

            size_t len = snprintf(buf, 4096, format,
                                  e->name,
                                  e->frame_id,
                                  time::get_sec(e->start),
                                  time::get_nsec(e->start),
                                  time::get_sec(e->end),
                                  time::get_nsec(e->end),
                                  e->worker_id,
                                  e->depth);

            array::push(buffer, buf, len);
        }

        static void recordfloat_to_json(const void* event,
                                        Array < char >& buffer) {
            RecordFloatEvent* e = (RecordFloatEvent*)event;

            static char buf[4096] = {0};
            static const char format[] = "    name: %s\n"
                                         "    value: %f\n";

            size_t len = snprintf(buf, 4096, format,
                                  e->name,
                                  e->value);

            array::push(buffer, buf, len);
        }
    }

    namespace develop_manager_globals {
        void init() {
            log::info("develop_manager_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) DevelopManagerData(memory_globals::default_allocator());

            register_type(EVENT_BEGIN_FRAME, "EVENT_BEGIN_FRAME", beginframe_to_json);
            register_type(EVENT_END_FRAME, "EVENT_END_FRAME", endframe_to_json);
            register_type(EVENT_SCOPE, "EVENT_SCOPE", begintask_to_json);
            register_type(EVENT_RECORD_FLOAT, "EVENT_RECORD_FLOAT", recordfloat_to_json);
        }

        void shutdown() {
            log::info("develop_manager_globals", "Shutdown");

            _globals.data->~DevelopManagerData();
            _globals = Globals();
        }
    }
}
