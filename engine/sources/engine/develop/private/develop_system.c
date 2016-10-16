//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>
#include <engine/core/memory.h>
#include <celib/os/thread_types.h>
#include <celib/os/thread.h>
#include <engine/core/task.h>
#include <engine/develop/develop_system.h>
#include <SDL2/SDL_timer.h>


#include "include/nanomsg/nn.h"
#include "include/nanomsg/pubsub.h"

#include "celib/errors/errors.h"
#include "celib/containers/eventstream.h"

#include "engine/core/types.h"
#include "engine/core/cvar.h"

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "develop_system"

//==============================================================================
// Globals
//==============================================================================

#define _G DevelopSystemGlobals

static struct G {
    struct eventstream eventstream;

    cvar_t cv_pub_port;
    cvar_t cv_pub_addr;
    int pub_socket;

    spinlock_t flush_lock;

} _G = {0};

static __thread u8 _stream_buffer[64 * 1024] = {0};
static __thread u32 _stream_buffer_size = 0;
static __thread u32 _scope_depth = 0;

static void _flush_stream_buffer() {
    os_thread_spin_lock(&_G.flush_lock);

    array_push_u8(&_G.eventstream.stream, _stream_buffer, _stream_buffer_size);
    _stream_buffer_size = 0;

    os_thread_spin_unlock(&_G.flush_lock);
}

static void _flush_all_job(void *data) {
    _flush_stream_buffer();
}

static void _flush_all_streams() {
    const int wc = taskmanager_worker_count() + 1;
    task_t tasks[wc + 1];

    tasks[0] = taskmanager_add_null("flush_all", task_null, task_null, TASK_PRIORITY_HIGH, TASK_AFFINITY_MAIN);

    for (int i = 0; i < wc; ++i) {
        tasks[i + 1] = taskmanager_add_begin("flush_worker", _flush_all_job, NULL, 0, task_null, tasks[0],
                                             TASK_PRIORITY_HIGH, TASK_AFFINITY_MAIN + i);
    }

    taskmanager_add_end(tasks, wc + 1);
    taskmanager_wait(tasks[0]);
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

    eventstream_create(&_G.eventstream, memsys_main_allocator());

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

    eventstream_destroy(&_G.eventstream);
    nn_close(_G.pub_socket);
}

void developsys_update() {
    _flush_all_streams();
}

void developsys_push_record_float(const char *name,
                                  float value) {
    struct record_float_event ev;
    ev.value = value;
    memory_copy(ev.name, name, str_lenght(name));

    developsys_push(EVENT_RECORD_FLOAT, ev);
}

void developsys_push_record_int(const char *name,
                                int value) {
    struct record_int_event ev;
    ev.value = value;
    memory_copy(ev.name, name, str_lenght(name));

    developsys_push(EVENT_RECORD_INT, ev);
}

time_t developsys_enter_scope(const char *name) {
    ++_scope_depth;
    return SDL_GetTicks(); // TODO: High performance counter
}

void developsys_leave_scope(const char *name,
                            time_t start_time){
    --_scope_depth;

    struct scope_event ev = {
            .worker_id = taskmanager_worker_id(),
            .start = start_time,
            .end = SDL_GetTicks(),
            .depth = _scope_depth,
    };

    memory_copy(ev.name, name, str_lenght(name));

    developsys_push(EVENT_SCOPE, ev);
}

