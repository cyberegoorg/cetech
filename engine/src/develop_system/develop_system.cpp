//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/mpack/mpack.h"
#include "include/nanomsg/nn.h"
#include "include/nanomsg/pubsub.h"

#include <celib/map.inl>

#include <cetech/machine/machine.h>
#include <celib/eventstream.inl>
#include <cetech/memory.h>
#include <cetech/config.h>
#include <cetech/log.h>

#include <cetech/task.h>
#include <cetech/develop.h>

#include <cetech/api_system.h>
#include <cetech/os/errors.h>
#include <cetech/os/thread.h>
#include <cetech/os/time.h>

using namespace celib;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_time_a0);
CETECH_DECL_API(ct_log_a0);


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "develop_system"
#define developsys_push(type, event) _developsys_push((ct_develop_event_header*)(&event), type, sizeof(event))

//==============================================================================
// Typedefs
//==============================================================================

typedef void (*to_mpack_fce_t)(const ct_develop_event_header *event,
                               mpack_writer_t *writer);

//==============================================================================
// Globals
//==============================================================================

#define _G DevelopSystemGlobals

static struct DevelopSystemGlobals {
    EventStream eventstream;
    Map<to_mpack_fce_t> to_mpack;

    ct_cvar cv_pub_addr;
    int pub_socket;

    ct_spinlock flush_lock;
    atomic_int complete_flag[8]; // TODO: dynamic
    float time_accum;

    void init(cel_alloc *a) {
        this->to_mpack.init(a);
        this->eventstream.init(a);
    }

    void shutdown() {
        this->eventstream.destroy();
        this->to_mpack.destroy();
    }
} _G;

//static __thread uint8_t _stream_buffer[64 * 1024] = {0};
//static __thread uint32_t _stream_buffer_size = 0;

static __thread uint32_t _scope_depth = 0;

//==============================================================================
// Internals
//==============================================================================

namespace {
    void _register_to_mpack(uint64_t type,
                            to_mpack_fce_t fce) {
        map::set(_G.to_mpack, type, fce);
    }

    static void _scopeevent_to_mpack(const ct_develop_event_header *event,
                                     mpack_writer_t *writer) {
        const ct_scope_event *e = (const ct_scope_event *) event;

        mpack_start_map(writer, 6);

        mpack_write_cstr(writer, "etype");
        mpack_write_cstr(writer, "EVENT_SCOPE");

        mpack_write_cstr(writer, "name");
        mpack_write_cstr(writer, e->name);

        mpack_write_cstr(writer, "start");
        mpack_write_i64(writer, e->start);

        mpack_write_cstr(writer, "duration");
        mpack_write_float(writer, e->duration);

        mpack_write_cstr(writer, "worker_id");
        mpack_write_i32(writer, e->worker_id);

        mpack_write_cstr(writer, "depth");
        mpack_write_i32(writer, e->depth);

        mpack_finish_map(writer);
    }

    static void _recordfloat_to_mpack(const ct_develop_event_header *event,
                                      mpack_writer_t *writer) {
        const ct_record_float_event *e = (const ct_record_float_event *) event;

        mpack_start_map(writer, 3);

        mpack_write_cstr(writer, "etype");
        mpack_write_cstr(writer, "EVENT_RECORD_FLOAT");

        mpack_write_cstr(writer, "name");
        mpack_write_cstr(writer, e->name);

        mpack_write_cstr(writer, "value");
        mpack_write_float(writer, e->value);

        mpack_finish_map(writer);
    }

    static void _recordint_to_mpack(const ct_develop_event_header *event,
                                    mpack_writer_t *writer) {
        const ct_record_int_event *e = (const ct_record_int_event *) event;

        mpack_start_map(writer, 3);

        mpack_write_cstr(writer, "etype");
        mpack_write_cstr(writer, "EVENT_RECORD_INT");

        mpack_write_cstr(writer, "name");
        mpack_write_cstr(writer, e->name);

        mpack_write_cstr(writer, "value");
        mpack_write_float(writer, e->value);

        mpack_finish_map(writer);
    }

    void _send_events() {
        uint32_t event_num = 0;

        auto *event = eventstream::begin(_G.eventstream);
        while (event != eventstream::end(_G.eventstream)) {
            ++event_num;
            event = eventstream::next(_G.eventstream, event);
        }

        char *data;
        size_t size;
        mpack_writer_t writer;
        mpack_writer_init_growable(&writer, &data, &size);

        mpack_start_array(&writer, event_num);

        event = eventstream::begin(_G.eventstream);
        while (event != eventstream::end(_G.eventstream)) {
            to_mpack_fce_t to_mpack_fce = map::get<to_mpack_fce_t>(_G.to_mpack,
                                                                   event->type,
                                                                   NULL);

            if (to_mpack_fce != NULL) {
                to_mpack_fce((const ct_develop_event_header *) event, &writer);
            }

            event = eventstream::next(_G.eventstream, event);
        }

        mpack_finish_array(&writer);
        CETECH_ASSERT("develop_manager",
                      mpack_writer_destroy(&writer) == mpack_ok);
        int bytes = nn_send(_G.pub_socket, data, size, 0);
        CETECH_ASSERT("develop", bytes == size);
        free(data);
    }

    static void _init_cvar(struct ct_config_a0 config) {
        _G = {0};
        _G.cv_pub_addr = config.new_str("develop.pub.addr",
                                        "Console server rpc addr",
                                        "ws://*:4447");


    }

}

//==============================================================================
// Interface
//==============================================================================
namespace develop_system {

    static void _after_update(float dt) {
        _G.time_accum += dt;
        if (_G.time_accum >= 10.0f / 1000.0f) {
            _send_events();

            _G.time_accum = 0.0f;
        }

        eventstream::clear(_G.eventstream);
    }

    void _developsys_push(ct_develop_event_header *header,
                          uint32_t type,
                          uint64_t size) {

        header->type = type;
        header->size = size;

        ct_thread_a0.spin_lock(&_G.flush_lock);

        array::push(_G.eventstream, (const uint8_t *) header, size);

        ct_thread_a0.spin_unlock(&_G.flush_lock);
    }

    void developsys_push_record_float(const char *name,
                                      float value) {
        ct_record_float_event ev = {0};

        ev.value = value;
        memcpy(ev.name, name, strlen(name));

        developsys_push(EVENT_RECORD_FLOAT, ev);
    }

    void developsys_push_record_int(const char *name,
                                    int value) {
        ct_record_int_event ev = {0};
        ev.value = value;
        memcpy(ev.name, name, strlen(name));

        developsys_push(EVENT_RECORD_INT, ev);
    }

    ct_scope_data developsys_enter_scope(const char *name, uint32_t worker_id) {
        ++_scope_depth;

        return (ct_scope_data) {
                .name = name,
                .start = ct_time_a0.ticks(),
                .start_timer = ct_time_a0.perf_counter(),
                .worker_id = worker_id
        };
    }

    void developsys_leave_scope(ct_scope_data scope_data) {
        CETECH_ASSERT(LOG_WHERE, _scope_depth > 0);
        --_scope_depth;

        ct_scope_event ev = {
                .name = {0},
                .worker_id = scope_data.worker_id,
                .start = scope_data.start,
                .duration =
                ((float) (ct_time_a0.perf_counter() -
                          scope_data.start_timer) /
                 ct_time_a0.perf_freq()) * 1000.0f,
                .depth = _scope_depth,
        };

        memcpy(ev.name, scope_data.name, strlen(scope_data.name));

        developsys_push(EVENT_SCOPE, ev);
    }
}

namespace develop_system_module {
    static ct_develop_a0 _api = {
            .push = develop_system::_developsys_push,
            .push_record_float = develop_system::developsys_push_record_float,
            .push_record_int = develop_system::developsys_push_record_int,
            .leave_scope = develop_system::developsys_leave_scope,
            .enter_scope = develop_system::developsys_enter_scope,
            .after_update = develop_system::_after_update
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_develop_a0", &_api);
    }

    void _init(ct_api_a0 *api) {
        _init_api(api);

        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_config_a0);

        CETECH_GET_API(api, ct_thread_a0);
        CETECH_GET_API(api, ct_time_a0);

        _init_cvar(ct_config_a0);

        _G.init(ct_memory_a0.main_allocator());

        _register_to_mpack(EVENT_SCOPE, _scopeevent_to_mpack);
        _register_to_mpack(EVENT_RECORD_FLOAT, _recordfloat_to_mpack);
        _register_to_mpack(EVENT_RECORD_INT, _recordint_to_mpack);

        const char *addr = 0;

        ct_log_a0.debug(LOG_WHERE, "Init");

        int socket = nn_socket(AF_SP, NN_PUB);
        if (socket < 0) {
            ct_log_a0.error(LOG_WHERE, "Could not create nanomsg socket: %s",
                            nn_strerror(errno));
            //return 0;
        }
        addr = ct_config_a0.get_string(_G.cv_pub_addr);

        ct_log_a0.debug(LOG_WHERE, "PUB address: %s", addr);

        if (nn_bind(socket, addr) < 0) {
            ct_log_a0.error(LOG_WHERE, "Could not bind socket to '%s': %s",
                            addr,
                            nn_strerror(errno));
            //return 0;
        }

        _G.pub_socket = socket;
    }

    void _shutdown() {
        ct_log_a0.debug(LOG_WHERE, "Shutdown");

        nn_close(_G.pub_socket);

        _G.shutdown();
    }

    extern "C" void developsystem_load_module(ct_api_a0 *api) {
        _init(api);

    }

    extern "C" void developsystem_unload_module(ct_api_a0 *api) {
        _shutdown();
    }
}