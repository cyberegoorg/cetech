//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <cetech/kernel/os.h>
#include <cetech/core/map.inl>

#include "include/mpack/mpack.h"
#include "include/nanomsg/nn.h"
#include "include/nanomsg/pubsub.h"

#include <cetech/kernel/thread.h>
#include <cetech/core/eventstream.inl>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/module.h>

#include <cetech/modules/task/task.h>
#include <cetech/modules/develop_system/develop.h>
#include <cetech/kernel/string.h>
#include <cetech/kernel/api.h>


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "develop_system"

//==============================================================================
// Globals
//==============================================================================

#define _G DevelopSystemGlobals
#define developsys_push(type, event) _developsys_push((struct event_header*)(&event), type, sizeof(event))

typedef void (*to_mpack_fce_t)(const struct event_header *event,
                               mpack_writer_t *writer);

ARRAY_PROTOTYPE(to_mpack_fce_t);
MAP_PROTOTYPE(to_mpack_fce_t);

static struct G {
    struct eventstream eventstream;
    MAP_T(to_mpack_fce_t) to_mpack;
    cvar_t cv_pub_addr;
    int pub_socket;

    spinlock_t flush_lock;
    atomic_int complete_flag[8]; // TODO: dynamic
    float time_accum;
} _G = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(thread_api_v0);
IMPORT_API(task_api_v0);
IMPORT_API(config_api_v0);
IMPORT_API(time_api_v0);

static __thread uint8_t _stream_buffer[64 * 1024] = {0};
static __thread uint32_t _stream_buffer_size = 0;
static __thread uint32_t _scope_depth = 0;

static void _flush_stream_buffer() {
    if (_stream_buffer_size == 0) {
        return;
    }

    thread_api_v0.spin_lock(&_G.flush_lock);

    array_push_uint8_t(&_G.eventstream.stream, _stream_buffer,
                       _stream_buffer_size);
    _stream_buffer_size = 0;

    thread_api_v0.spin_unlock(&_G.flush_lock);
}

static void _flush_job(void *data) {
    _flush_stream_buffer();

    atomic_store_explicit(&_G.complete_flag[task_api_v0.worker_id()], 1,
                          memory_order_release);
}

static void _flush_all_streams() {
    const int wc = task_api_v0.worker_count();

    for (int i = 1; i < wc; ++i) {
        atomic_init(&_G.complete_flag[i], 0);
    }

    _flush_stream_buffer();

    struct task_item items[wc];

    for (int i = 0; i < wc; ++i) {
        items[i] = (struct task_item) {
                .name = "flush_worker",
                .work = _flush_job,
                .data = NULL,
                .affinity = TASK_AFFINITY_WORKER1 + i
        };
    }

    task_api_v0.add(items, wc);

    for (int i = 1; i < wc; ++i) {
        task_api_v0.wait_atomic(&_G.complete_flag[i], 0);
    }
}

void _developsys_push(struct event_header *header,
                      uint32_t type,
                      uint64_t size) {

    if ((_stream_buffer_size + size) >= CETECH_ARRAY_LEN(_stream_buffer)) {
        _flush_stream_buffer();
    }

    uint8_t *p = _stream_buffer + _stream_buffer_size;

    header->type = type;
    header->size = size;

    memcpy(p, header, size);
    _stream_buffer_size += size;
}

void _register_to_mpack(uint64_t type,
                        to_mpack_fce_t fce) {
    MAP_SET(to_mpack_fce_t, &_G.to_mpack, type, fce);
}

static void _scopeevent_to_mpack(const struct event_header *event,
                                 mpack_writer_t *writer) {
    const struct scope_event *e = (const struct scope_event *) event;

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

static void _recordfloat_to_mpack(const struct event_header *event,
                                  mpack_writer_t *writer) {
    const struct record_float_event *e = (const struct record_float_event *) event;

    mpack_start_map(writer, 3);

    mpack_write_cstr(writer, "etype");
    mpack_write_cstr(writer, "EVENT_RECORD_FLOAT");

    mpack_write_cstr(writer, "name");
    mpack_write_cstr(writer, e->name);

    mpack_write_cstr(writer, "value");
    mpack_write_float(writer, e->value);

    mpack_finish_map(writer);
}

static void _recordint_to_mpack(const struct event_header *event,
                                mpack_writer_t *writer) {
    const struct record_int_event *e = (const struct record_int_event *) event;

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

    struct event_header *event = eventstream_begin(&_G.eventstream);
    while (event != eventstream_end(&_G.eventstream)) {
        ++event_num;
        event = eventstream_next(event);
    }

    char *data;
    size_t size;
    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &data, &size);

    mpack_start_array(&writer, event_num);

    event = eventstream_begin(&_G.eventstream);
    while (event != eventstream_end(&_G.eventstream)) {
        to_mpack_fce_t to_mpack_fce = MAP_GET(to_mpack_fce_t, &_G.to_mpack,
                                              event->type, NULL);

        if (to_mpack_fce != NULL) {
            to_mpack_fce(event, &writer);
        }

        event = eventstream_next(event);
    }

    mpack_finish_array(&writer);
    CETECH_ASSERT("develop_manager", mpack_writer_destroy(&writer) == mpack_ok);
    int bytes = nn_send(_G.pub_socket, data, size, 0);
    CETECH_ASSERT("develop", bytes == size);
    free(data);
}

//==============================================================================
// Interface
//==============================================================================

static void _init_cvar(struct config_api_v0 config) {
    _G = (struct G) {0};
    _G.cv_pub_addr = config.new_str("develop.pub.addr",
                                    "Console server rpc addr", "ws://*:4447");


}


static void _after_update(float dt) {
    _flush_all_streams();

    _G.time_accum += dt;
    if (_G.time_accum >= 10.0f / 1000.0f) {
        _send_events();

        _G.time_accum = 0.0f;
    }

    eventstream_clear(&_G.eventstream);
}

void developsys_push_record_float(const char *name,
                                  float value) {
    struct record_float_event ev = {0};

    ev.value = value;
    memcpy(ev.name, name, strlen(name));

    developsys_push(EVENT_RECORD_FLOAT, ev);
}

void developsys_push_record_int(const char *name,
                                int value) {
    struct record_int_event ev = {0};
    ev.value = value;
    memcpy(ev.name, name, strlen(name));

    developsys_push(EVENT_RECORD_INT, ev);
}

struct scope_data developsys_enter_scope(const char *name) {
    ++_scope_depth;

    return (struct scope_data) {
            .name = name,
            .start = time_api_v0.get_ticks(),
            .start_timer = time_api_v0.get_perf_counter()
    };
}

void developsys_leave_scope(struct scope_data scope_data) {
    CETECH_ASSERT(LOG_WHERE, _scope_depth > 0);
    --_scope_depth;

    struct scope_event ev = {
            .name = {0},
            .worker_id = task_api_v0.worker_id(),
            .start = scope_data.start,
            .duration =
            ((float) (time_api_v0.get_perf_counter() - scope_data.start_timer) /
             time_api_v0.get_perf_freq()) * 1000.0f,
            .depth = _scope_depth,
    };

    memcpy(ev.name, scope_data.name, strlen(scope_data.name));

    developsys_push(EVENT_SCOPE, ev);
}

static void _init_api(struct api_v0* api){
    static struct develop_api_v0 _api = {0};

    _api.push = _developsys_push;
    _api.push_record_float = developsys_push_record_float;
    _api.push_record_int = developsys_push_record_int;
    _api.leave_scope = developsys_leave_scope;
    _api.enter_scope = developsys_enter_scope;

    api->register_api("develop_api_v0", &_api);
}

static void _init( struct api_v0* api) {
    USE_API(api, memory_api_v0);
    USE_API(api, task_api_v0);
    USE_API(api, config_api_v0);
    USE_API(api, thread_api_v0);
    USE_API(api, time_api_v0);

    MAP_INIT(to_mpack_fce_t, &_G.to_mpack, memory_api_v0.main_allocator());
    eventstream_create(&_G.eventstream, memory_api_v0.main_allocator());

    _register_to_mpack(EVENT_SCOPE, _scopeevent_to_mpack);
    _register_to_mpack(EVENT_RECORD_FLOAT, _recordfloat_to_mpack);
    _register_to_mpack(EVENT_RECORD_INT, _recordint_to_mpack);

    const char *addr = 0;

    log_debug(LOG_WHERE, "Init");

    int socket = nn_socket(AF_SP, NN_PUB);
    if (socket < 0) {
        log_error(LOG_WHERE, "Could not create nanomsg socket: %s",
                  nn_strerror(errno));
        //return 0;
    }
    addr = config_api_v0.get_string(_G.cv_pub_addr);

    log_debug(LOG_WHERE, "PUB address: %s", addr);

    if (nn_bind(socket, addr) < 0) {
        log_error(LOG_WHERE, "Could not bind socket to '%s': %s", addr,
                  nn_strerror(errno));
        //return 0;
    }

    _G.pub_socket = socket;
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    MAP_DESTROY(to_mpack_fce_t, &_G.to_mpack);

    eventstream_destroy(&_G.eventstream);
    nn_close(_G.pub_socket);
}


void *developsystem_get_module_api(int api) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;
            module.init_cvar = _init_cvar;
            module.after_update = _after_update;

            return &module;
        }


        default:
            return NULL;
    }

    return 0;
}
