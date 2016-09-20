#include "celib/macros.h"
#include "celib/types.h"
#include "celib/memory/memory.h"
#include "celib/container/hash.inl.h"
#include "celib/container/eventstream.inl.h"

#include "cetech/develop/develop_manager.h"
#include "cetech/develop/console_server.h"
#include "cetech/application/application.h"

#include "celib/log/log.h"
#include "celib/thread/thread.h"

#include "rapidjson/document.h"

#include "nanomsg/nn.h"
#include "nanomsg/pubsub.h"

#include "mpack/mpack.h"

namespace cetech {
    namespace {
        using namespace develop_manager;

        typedef void (* to_mpack_fce_t)(const void*,
                                       mpack_writer_t* writer);

        static thread_local char _stream_buffer[64 * 1024] = {0};
        static thread_local uint32_t _stream_buffer_size = 0;

        static thread_local uint32_t _scope_depth = 0;

        struct DevelopManagerData {
            Hash < to_mpack_fce_t > to_mpack;
            Hash < const char* > type_to_string;
            Array < char > buffer;
            EventStream stream;
            Spinlock lock;
            int dev_pub_socket;

            explicit DevelopManagerData(Allocator& allocator) : to_mpack(allocator),
                                                                type_to_string(allocator),
                                                                buffer(allocator),
                                                                stream(allocator),
                                                                dev_pub_socket(0) {
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
            //log::info("dm", "push: %d %d\n", type, _stream_buffer_size);
            const uint32_t sz = sizeof(eventstream::Header) + sizeof(T);

            if ((_stream_buffer_size + sz) >= 64 * 1024) {
                flush_stream_buffer();
            }

            char* p = _stream_buffer + _stream_buffer_size;

            eventstream::Header* h = (eventstream::Header*)p;
            h->type = type;
            h->size = sizeof(T);

            _stream_buffer_size += sz;

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

            eventstream::write(develop_manager::event_stream(), _stream_buffer, _stream_buffer_size);
            _stream_buffer_size = 0;

            thread::spin_unlock(_globals.data->lock);
        }

        void register_type(EventType type,
                           const char* type_str,
                           to_mpack_fce_t fce) {
            hash::set(_globals.data->to_mpack, type, fce);
            hash::set(_globals.data->type_to_string, type, type_str);
        }

        void send_buffer() {
            if (eventstream::empty(_globals.data->stream)) {
                return;
            }

            thread::spin_lock(_globals.data->lock); // TODO: remove

            uint32_t event_num = 0;
            eventstream::event_it it = 0;
            while (eventstream::valid(_globals.data->stream, it)) {
                event_num += 1;
                it = eventstream::next(_globals.data->stream, it);
            }

            char* data;
            size_t size;
            mpack_writer_t writer;
            mpack_writer_init_growable(&writer, &data, &size);

            mpack_start_array(&writer, event_num);

            it = 0;
            while (eventstream::valid(_globals.data->stream, it)) {
                eventstream::Header* header = eventstream::header(_globals.data->stream, it);

                //etype
                to_mpack_fce_t to_mpack_fce = hash::get < to_mpack_fce_t > (_globals.data->to_mpack, header->type, nullptr);
                CE_ASSERT("develop_manager", to_mpack_fce);

                to_mpack_fce(eventstream::event < void* > (_globals.data->stream, it), &writer);

                it = eventstream::next(_globals.data->stream, it);
            }

            thread::spin_unlock(_globals.data->lock);
            mpack_finish_array(&writer);

            CE_ASSERT("develop_manager", mpack_writer_destroy(&writer) == mpack_ok);

            int socket = _globals.data->dev_pub_socket;
            size_t bytes = nn_send(socket, data, size, 0);
            CE_ASSERT("develop", bytes == size);
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

        static void beginframe_to_mpack(const void* event,
                                       mpack_writer_t* writer) {
            BeginFrameEvent* e = (BeginFrameEvent*)event;

            mpack_start_map(writer, 4);

            mpack_write_cstr(writer, "etype");
            mpack_write_cstr(writer, "EVENT_BEGIN_FRAME");

            mpack_write_cstr(writer, "time");
            mpack_write_i64(writer, time::get_sec(e->time));

            mpack_write_cstr(writer, "time_ns");
            mpack_write_i64(writer, time::get_nsec(e->time));

            mpack_write_cstr(writer, "frame_id");
            mpack_write_i64(writer, e->frame_id);

            mpack_finish_map(writer);
        }

        static void endframe_to_mpack(const void* event,
                                     mpack_writer_t* writer) {
            EndFrameEvent* e = (EndFrameEvent*)event;

            mpack_start_map(writer, 4);

            mpack_write_cstr(writer, "etype");
            mpack_write_cstr(writer, "EVENT_END_FRAME");

            mpack_write_cstr(writer, "time");
            mpack_write_i64(writer, time::get_sec(e->time));

            mpack_write_cstr(writer, "time_ns");
            mpack_write_i64(writer, time::get_nsec(e->time));

            mpack_write_cstr(writer, "frame_id");
            mpack_write_i64(writer, e->frame_id);

            mpack_finish_map(writer);
        }

        static void begintask_to_mpack(const void* event,
                                      mpack_writer_t* writer) {
            ScopeEvent* e = (ScopeEvent*)event;

            mpack_start_map(writer, 9);

            mpack_write_cstr(writer, "etype");
            mpack_write_cstr(writer, "EVENT_SCOPE");

            mpack_write_cstr(writer, "name");
            mpack_write_cstr(writer, e->name);

            mpack_write_cstr(writer, "frame_id");
            mpack_write_i64(writer, e->frame_id);

            mpack_write_cstr(writer, "start");
            mpack_write_i64(writer, time::get_sec(e->start));

            mpack_write_cstr(writer, "start_ns");
            mpack_write_i64(writer, time::get_nsec(e->start));

            mpack_write_cstr(writer, "end");
            mpack_write_i64(writer, time::get_sec(e->end));

            mpack_write_cstr(writer, "end_ns");
            mpack_write_i64(writer, time::get_nsec(e->end));

            mpack_write_cstr(writer, "worker_id");
            mpack_write_i32(writer, e->worker_id);

            mpack_write_cstr(writer, "depth");
            mpack_write_i32(writer, e->depth);

            mpack_finish_map(writer);
        }

        static void recordfloat_to_mpack(const void* event,
                                        mpack_writer_t* writer) {
            RecordFloatEvent* e = (RecordFloatEvent*)event;

            mpack_start_map(writer, 4);

            mpack_write_cstr(writer, "etype");
            mpack_write_cstr(writer, "EVENT_RECORD_FLOAT");

            mpack_write_cstr(writer, "name");
            mpack_write_cstr(writer, e->name);

            mpack_write_cstr(writer, "value");
            mpack_write_float(writer, e->value);
                        
            mpack_write_cstr(writer, "frame_id");
            mpack_write_i64(writer, e->frame_id);
            
            mpack_finish_map(writer);
        }
    }

    namespace develop_manager_globals {
        void init() {
            log::info("develop_manager_globals", "Init");

            char* p = _globals.buffer;
            _globals.data = new(p) DevelopManagerData(memory_globals::default_allocator());

            register_type(EVENT_BEGIN_FRAME, "EVENT_BEGIN_FRAME", beginframe_to_mpack);
            register_type(EVENT_END_FRAME, "EVENT_END_FRAME", endframe_to_mpack);
            register_type(EVENT_SCOPE, "EVENT_SCOPE", begintask_to_mpack);
            register_type(EVENT_RECORD_FLOAT, "EVENT_RECORD_FLOAT", recordfloat_to_mpack);

            int socket = nn_socket(AF_SP, NN_PUB);
            CE_ASSERT("develop_manager", socket >= 0);
            CE_ASSERT("develop_manager", nn_bind(socket, "ws://*:5558") >= 0);
            _globals.data->dev_pub_socket = socket;
        }

        void shutdown() {
            log::info("develop_manager_globals", "Shutdown");

            _globals.data->~DevelopManagerData();
            _globals = Globals();
        }
    }
}
