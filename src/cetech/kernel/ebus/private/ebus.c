//==============================================================================
// Includes
//==============================================================================


#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/os/thread.h>
#include <cetech/kernel/os/cpu.h>
#include <cetech/kernel/log/log.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/kernel/macros.h>
#include <cetech/kernel/containers/hash.h>
#include <cetech/kernel/hashlib/hashlib.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_thread_a0);
CETECH_DECL_API(ct_cpu_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "ebus"
#define _G EBusGlobal

//==============================================================================
// Globals
//==============================================================================

struct ebus_event_handler {
    uint64_t addr;
    uint32_t order;
    ct_ebus_handler *handler;
};

struct ebus_event_handlers {
    struct ebus_event_handler *handlers;
};

struct ebus_t {
    uint32_t bus_name;

    struct ct_hash_t handler_idx;
    struct ebus_event_handlers *handlers;
    uint8_t *buffer;
};

static struct _G {
    struct ct_hash_t ebus_idx;
    struct ebus_t *ebus_pool;

    struct ct_alloc *allocator;
} _G;


//==============================================================================
// Api
//==============================================================================


void create_ebus(const char *name,
                 uint32_t id) {
    uint64_t ebus_idx = ct_array_size(_G.ebus_pool);

    struct ebus_t ebus = {
            .bus_name = id,
    };

    ct_array_push(_G.ebus_pool, ebus, _G.allocator);
    ct_hash_add(&_G.ebus_idx, ebus.bus_name, ebus_idx, _G.allocator);
}

void send_addr(uint32_t bus_name,
               uint64_t event_type,
               uint64_t addr,
               void *event,
               uint64_t event_size) {

    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    uint32_t event_full_size = sizeof(struct ebus_header_t) + event_size;
    uint8_t eventdata[event_full_size];
    struct ebus_header_t *ev = (struct ebus_header_t *) eventdata;

    ev->type = event_type;
    ev->size = event_size;
    memcpy(ev + 1, event, event_size);

    uint64_t ev_offset = ct_array_size(ebus->buffer);
    ct_array_push_n(ebus->buffer, eventdata, event_full_size, _G.allocator);

    uint64_t event_idx = ct_hash_lookup(&ebus->handler_idx, event_type,
                                        UINT64_MAX);
    if (UINT64_MAX == event_idx) {
        return;
    }
    struct ebus_event_handlers *ev_handlers = &ebus->handlers[event_idx];

    const uint32_t handlers_n = ct_array_size(ev_handlers->handlers);
    for (int i = 0; i < handlers_n; ++i) {
        if (ev_handlers->handlers[i].addr &&
            (ev_handlers->handlers[i].addr != addr)) {
            continue;
        }

        ev_handlers->handlers[i].handler(bus_name,
                                         ebus->buffer + ev_offset +
                                         sizeof(struct ebus_header_t));
    }
}

void broadcast(uint32_t bus_name,
               uint64_t event_type,
               void *event,
               uint64_t event_size) {
    send_addr(bus_name, event_type, 0, event, event_size);
}

void begin_frame() {
    uint32_t ebus_n = ct_array_size(_G.ebus_pool);
    for (int i = 0; i < ebus_n; ++i) {
        struct ebus_t *ebus = &_G.ebus_pool[i];
        if (ct_array_any(ebus->buffer)) {
            ct_array_resize(ebus->buffer, 0, _G.allocator);
        }
    }
}

void _connect_addr(uint32_t bus_name,
                   uint64_t event,
                   uint64_t addr,
                   ct_ebus_handler *handler,
                   uint32_t order) {

    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    uint64_t event_idx = ct_hash_lookup(&ebus->handler_idx, event, UINT64_MAX);

    if (UINT64_MAX == event_idx) {
        event_idx = ct_array_size(ebus->handlers);

        struct ebus_event_handlers ev_handlers = {};
        ct_array_push(ebus->handlers, ev_handlers, _G.allocator);

        ct_hash_add(&ebus->handler_idx, event, event_idx, _G.allocator);
    }

    struct ebus_event_handlers *ev_handlers = &ebus->handlers[event_idx];

    struct ebus_event_handler h = {
            .handler = handler,
            .addr = addr,
            .order = order,
    };

    const uint32_t handlers_n = ct_array_size(ev_handlers->handlers);

    if(0 == handlers_n) {
        ct_array_push(ev_handlers->handlers, h, _G.allocator);
        return;
    }

    for (int i = 0; i < handlers_n; ++i) {
        if(ev_handlers->handlers[i].order < order) {
            continue;
        }

        ct_array_insert(ev_handlers->handlers, i, h, _G.allocator);
        return;
    }
}

void _connect(uint32_t bus_name,
              uint64_t event,
              ct_ebus_handler *handler,
              uint32_t order) {
    _connect_addr(bus_name, event, 0, handler, order);
}


void *first_event(uint32_t bus_name) {
    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return NULL;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    if (!ct_array_size(ebus->buffer)) {
        return NULL;
    }

    return ebus->buffer + sizeof(struct ebus_header_t);
}

void *next_event(uint32_t bus_name,
                 void *event) {
    struct ebus_header_t *header = event - sizeof(struct ebus_header_t);

    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return NULL;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];


    void *last = ebus->buffer + ct_array_size(ebus->buffer);
    void *next = event + header->size + sizeof(struct ebus_header_t);

    if (next >= last) {
        return NULL;
    }

    return next;
}

struct ebus_header_t *event_header(void *event) {
    struct ebus_header_t *header = event - sizeof(struct ebus_header_t);
    return header;
}

static struct ct_ebus_a0 _api = {
        .create_ebus = create_ebus,
        .broadcast = broadcast,
        .send = send_addr,
        .connect = _connect,
        .connect_addr = _connect_addr,
        .begin_frame = begin_frame,

        .first_event = first_event,
        .next_event = next_event,
        .event_header = event_header,
};

static void _init(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_thread_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_cpu_a0);
    CETECH_GET_API(api, ct_hashlib_a0);


    api->register_api("ct_ebus_a0", &_api);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator()
    };

    create_ebus("", 0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CETECH_MODULE_DEF(
        ebus,
        {
            CT_UNUSED(api);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)
