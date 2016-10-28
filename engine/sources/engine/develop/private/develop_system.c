//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <celib/os/time.h>
#include <celib/containers/map.h>

#include "include/SDL2/SDL_timer.h"
#include "include/mpack/mpack.h"
#include "include/nanomsg/nn.h"
#include "include/nanomsg/pubsub.h"

#include "celib/os/thread_types.h"
#include "celib/os/thread.h"
#include "celib/errors/errors.h"
#include "celib/containers/eventstream.h"

#include "celib/config/cvar.h"
#include "celib/task/task.h"
#include "celib/memory/memory.h"
#include "engine/develop/develop_system.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "develop_system"

//==============================================================================
// Globals
//==============================================================================

#define _G DevelopSystemGlobals

typedef void (*to_mpack_fce_t)(const struct event_header *event,
                               mpack_writer_t *writer);

ARRAY_PROTOTYPE(to_mpack_fce_t);
MAP_PROTOTYPE(to_mpack_fce_t);

static struct G {
    struct eventstream eventstream;
    MAP_T(to_mpack_fce_t) to_mpack;
    cvar_t cv_pub_port;
    cvar_t cv_pub_addr;
    int pub_socket;

    spinlock_t flush_lock;
    atomic_int complete_flag[8]; // TODO: dynamic
} _G = {0};

static __thread u8 _stream_buffer[64 * 1024] = {0};
static __thread u32 _stream_buffer_size = 0;
static __thread u32 _scope_depth = 0;

static void _flush_stream_buffer() {
    if (_stream_buffer_size == 0) {
        return;
    }

    os_thread_spin_lock(&_G.flush_lock);

    array_push_u8(&_G.eventstream.stream, _stream_buffer, _stream_buffer_size);
    _stream_buffer_size = 0;

    os_thread_spin_unlock(&_G.flush_lock);
}

static void _flush_job(void *data) {
    _flush_stream_buffer();

   atomic_store_explicit(&_G.complete_flag[taskmanager_worker_id()], 1, memory_order_release);
}

static void _flush_all_streams() {
    const int wc = taskmanager_worker_count();

    for(int i = 1; i < wc; ++i) {
        atomic_init(&_G.complete_flag[i], 0);
    }

    _flush_stream_buffer();

    struct task_item items[wc];

    for (int i = 0; i < wc; ++i) {
        items[i] = (struct task_item){
                .name = "flush_worker",
                .work = _flush_job,
                .data = NULL,
                .affinity = TASK_AFFINITY_WORKER1 + i
        };
    }

    taskmanager_add(items, wc);

    for (int i = 1; i < wc; ++i) {
        taskmanager_wait_atomic(&_G.complete_flag[i], 0);
    }
}

void _developsys_push(struct event_header *header,
                      u32 type,
                      u64 size) {

    if ((_stream_buffer_size + size) >= CE_ARRAY_LEN(_stream_buffer)) {
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
        to_mpack_fce_t to_mpack_fce = MAP_GET(to_mpack_fce_t, &_G.to_mpack, event->type, NULL);

        if (to_mpack_fce != NULL) {
            to_mpack_fce(event, &writer);
        }

        event = eventstream_next(event);
    }

    mpack_finish_array(&writer);
    CE_ASSERT("develop_manager", mpack_writer_destroy(&writer) == mpack_ok);
    int bytes = nn_send(_G.pub_socket, data, size, 0);
    CE_ASSERT("develop", bytes == size);
    free(data);
}

//==============================================================================
// Interface
//==============================================================================

int developsys_init(int stage) {
    if (stage == 0) {
        _G = (struct G) {0};

        _G.cv_pub_port = cvar_new_int("develop.pub.port", "Console server rpc port", 4447);
        _G.cv_pub_addr = cvar_new_str("develop.pub.addr", "Console server rpc addr", "ws://*");

        return 1;
    }

    MAP_INIT(to_mpack_fce_t, &_G.to_mpack, memsys_main_allocator());
    eventstream_create(&_G.eventstream, memsys_main_allocator());

    _register_to_mpack(EVENT_SCOPE, _scopeevent_to_mpack);
    _register_to_mpack(EVENT_RECORD_FLOAT, _recordfloat_to_mpack);
    _register_to_mpack(EVENT_RECORD_INT, _recordint_to_mpack);

    char addr[128] = {0};

    log_debug(LOG_WHERE, "Init");

    int socket = nn_socket(AF_SP, NN_PUB);
    if (socket < 0) {
        log_error(LOG_WHERE, "Could not create nanomsg socket: %s", nn_strerror(errno));
        return 0;
    }
    snprintf(addr, 128, "%s:%d", cvar_get_string(_G.cv_pub_addr), cvar_get_int(_G.cv_pub_port));

    log_debug(LOG_WHERE, "PUB address: %s", addr);

    if (nn_bind(socket, addr) < 0) {
        log_error(LOG_WHERE, "Could not bind socket to '%s': %s", addr, nn_strerror(errno));
        return 0;
    }

    _G.pub_socket = socket;

    return 1;
}

void developsys_shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    MAP_DESTROY(to_mpack_fce_t, &_G.to_mpack);

    eventstream_destroy(&_G.eventstream);
    nn_close(_G.pub_socket);
}

void developsys_update() {
    _flush_all_streams();
    _send_events();
    eventstream_clear(&_G.eventstream);
}

void developsys_push_record_float(const char *name,
                                  float value) {
    struct record_float_event ev = {0};

    ev.value = value;
    memory_copy(ev.name, name, str_lenght(name));

    developsys_push(EVENT_RECORD_FLOAT, ev);
}

void developsys_push_record_int(const char *name,
                                int value) {
    struct record_int_event ev = {0};
    ev.value = value;
    memory_copy(ev.name, name, str_lenght(name));

    developsys_push(EVENT_RECORD_INT, ev);
}

struct scope_data developsys_enter_scope(const char *name) {
    ++_scope_depth;

    return (struct scope_data) {
            .start = os_get_ticks(),
            .start_timer = os_get_perf_counter()
    };
}

void developsys_leave_scope(const char *name,
                            struct scope_data scope_data) {
    --_scope_depth;

    struct scope_event ev = {
            .name = {0},
            .worker_id = taskmanager_worker_id(),
            .start = scope_data.start,
            .duration = ((float) (os_get_perf_counter() - scope_data.start_timer) / os_get_perf_freq()) * 1000.0f,
            .depth = _scope_depth,
    };

    memory_copy(ev.name, name, str_lenght(name));

    developsys_push(EVENT_SCOPE, ev);
}

