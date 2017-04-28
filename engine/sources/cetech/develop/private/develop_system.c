//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <celib/time.h>
#include <celib/map.h>

#include "include/mpack/mpack.h"
#include "include/nanomsg/nn.h"
#include "include/nanomsg/pubsub.h"

#include "celib/thread.h"
#include "celib/eventstream.h"
#include <cetech/memory/memory.h>
#include <cetech/module/module.h>

#include <cetech/task/task.h>
#include <cetech/develop/develop.h>
#include <celib/string.h>


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

IMPORT_API(MemSysApi, 0);
IMPORT_API(TaskApi, 0);
IMPORT_API(ConfigApi, 0);

static __thread u8 _stream_buffer[64 * 1024] = {0};
static __thread u32 _stream_buffer_size = 0;
static __thread u32 _scope_depth = 0;

static void _flush_stream_buffer() {
    if (_stream_buffer_size == 0) {
        return;
    }

    cel_thread_spin_lock(&_G.flush_lock);

    array_push_u8(&_G.eventstream.stream, _stream_buffer, _stream_buffer_size);
    _stream_buffer_size = 0;

    cel_thread_spin_unlock(&_G.flush_lock);
}

static void _flush_job(void *data) {
    _flush_stream_buffer();

    atomic_store_explicit(&_G.complete_flag[TaskApiV0.worker_id()], 1,
                          memory_order_release);
}

static void _flush_all_streams() {
    const int wc = TaskApiV0.worker_count();

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

    TaskApiV0.add(items, wc);

    for (int i = 1; i < wc; ++i) {
        TaskApiV0.wait_atomic(&_G.complete_flag[i], 0);
    }
}

void _developsys_push(struct event_header *header,
                      u32 type,
                      u64 size) {

    if ((_stream_buffer_size + size) >= CEL_ARRAY_LEN(_stream_buffer)) {
        _flush_stream_buffer();
    }

    u8 *p = _stream_buffer + _stream_buffer_size;

    header->type = type;
    header->size = size;

    memory_copy(p, header, size);
    _stream_buffer_size += size;
}

void _register_to_mpack(u64 type,
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
    CEL_ASSERT("develop_manager", mpack_writer_destroy(&writer) == mpack_ok);
    int bytes = nn_send(_G.pub_socket, data, size, 0);
    CEL_ASSERT("develop", bytes == size);
    free(data);
}

//==============================================================================
// Interface
//==============================================================================
static void _init(get_api_fce_t get_engine_api) {
    INIT_API(MemSysApi, MEMORY_API_ID, 0);
    INIT_API(TaskApi, TASK_API_ID, 0);
    INIT_API(ConfigApi, CONFIG_API_ID, 0);

    MAP_INIT(to_mpack_fce_t, &_G.to_mpack, MemSysApiV0.main_allocator());
    eventstream_create(&_G.eventstream, MemSysApiV0.main_allocator());

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
    addr = ConfigApiV0.get_string(_G.cv_pub_addr);

    log_debug(LOG_WHERE, "PUB address: %s", addr);

    if (nn_bind(socket, addr) < 0) {
        log_error(LOG_WHERE, "Could not bind socket to '%s': %s", addr,
                  nn_strerror(errno));
        //return 0;
    }

    _G.pub_socket = socket;
}

static void _init_cvar(struct ConfigApiV0 config) {
    _G = (struct G) {0};
    _G.cv_pub_addr = config.new_str("develop.pub.addr",
                                    "Console server rpc addr", "ws://*:4447");
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    MAP_DESTROY(to_mpack_fce_t, &_G.to_mpack);

    eventstream_destroy(&_G.eventstream);
    nn_close(_G.pub_socket);
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
    memory_copy(ev.name, name, cel_strlen(name));

    developsys_push(EVENT_RECORD_FLOAT, ev);
}

void developsys_push_record_int(const char *name,
                                int value) {
    struct record_int_event ev = {0};
    ev.value = value;
    memory_copy(ev.name, name, cel_strlen(name));

    developsys_push(EVENT_RECORD_INT, ev);
}

struct scope_data developsys_enter_scope(const char *name) {
    ++_scope_depth;

    return (struct scope_data) {
            .name = name,
            .start = cel_get_ticks(),
            .start_timer = cel_get_perf_counter()
    };
}

void developsys_leave_scope(struct scope_data scope_data) {
    CEL_ASSERT(LOG_WHERE, _scope_depth > 0);
    --_scope_depth;

    struct scope_event ev = {
            .name = {0},
            .worker_id = TaskApiV0.worker_id(),
            .start = scope_data.start,
            .duration =
            ((float) (cel_get_perf_counter() - scope_data.start_timer) /
             cel_get_perf_freq()) * 1000.0f,
            .depth = _scope_depth,
    };

    memory_copy(ev.name, scope_data.name, cel_strlen(scope_data.name));

    developsys_push(EVENT_SCOPE, ev);
}

void *developsystem_get_module_api(int api,
                                   int version) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;
                    module.init_cvar = _init_cvar;
                    module.after_update = _after_update;

                    return &module;
                }

                default:
                    return NULL;
            };
        case DEVELOP_SERVER_API_ID:
            switch (version) {
                case 0: {
                    static struct DevelopSystemApiV0 api = {0};

                    api.push = _developsys_push;
                    api.push_record_float = developsys_push_record_float;
                    api.push_record_int = developsys_push_record_int;
                    api.leave_scope = developsys_leave_scope;
                    api.enter_scope = developsys_enter_scope;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }

    return 0;
}
