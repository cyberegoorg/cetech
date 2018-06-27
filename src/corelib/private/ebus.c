//==============================================================================
// Includes
//==============================================================================


#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <corelib/os.h>
#include <corelib/log.h>
#include <corelib/module.h>
#include <corelib/ebus.h>
#include <corelib/macros.h>
#include <corelib/hash.inl>
#include <corelib/hashlib.h>

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
    uint64_t *events;
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
               uint64_t addr,
               uint64_t event) {

    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    ct_array_push(ebus->events, event, _G.allocator);

    uint64_t event_type = ct_cdb_a0->type(event);

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

        ev_handlers->handlers[i].handler(event);
    }
}

void broadcast(uint32_t bus_name,
               uint64_t event) {
    send_addr(bus_name, 0, event);
}

void begin_frame() {
    uint32_t ebus_n = ct_array_size(_G.ebus_pool);
    for (int i = 0; i < ebus_n; ++i) {
        struct ebus_t *ebus = &_G.ebus_pool[i];
        if (ct_array_any(ebus->events)) {
            for (int j = 0; j < ct_array_size(ebus->events); ++j) {
                ct_cdb_a0->destroy_object(ebus->events[j]);
            }

            ct_array_resize(ebus->events, 0, _G.allocator);
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

    if (0 == handlers_n) {
        ct_array_push(ev_handlers->handlers, h, _G.allocator);
        return;
    }

    for (int i = 0; i < handlers_n; ++i) {
        if (ev_handlers->handlers[i].order < order) {
            continue;
        }

        ct_array_insert(ev_handlers->handlers, i, h, _G.allocator);
        return;
    }

    ct_array_push(ev_handlers->handlers, h, _G.allocator);
}

void _connect(uint32_t bus_name,
              uint64_t event,
              ct_ebus_handler *handler,
              uint32_t order) {
    _connect_addr(bus_name, event, 0, handler, order);
}

void disconnect_addr(uint32_t bus_name,
                     uint64_t event,
                     uint64_t addr,
                     ct_ebus_handler *handler) {
    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    uint64_t event_idx = ct_hash_lookup(&ebus->handler_idx, event, UINT64_MAX);

    if (UINT64_MAX == event_idx) {
        return;
    }

    struct ebus_event_handlers *ev_handlers = &ebus->handlers[event_idx];
    const uint32_t handlers_n = ct_array_size(ev_handlers->handlers);

    if (0 == handlers_n) {
        return;
    }

    for (int i = 0; i < handlers_n; ++i) {
        if (ev_handlers->handlers[i].addr != addr) {
            continue;
        }

        if (ev_handlers->handlers[i].handler != handler) {
            continue;
        }

        memcpy(ev_handlers->handlers + i, ev_handlers->handlers + i + 1,
               sizeof(struct ebus_event_handler) * (handlers_n - (i)));
        ct_array_pop_back(ev_handlers->handlers);
        return;
    }
}

void disconnect(uint32_t bus_name,
                uint64_t event,
                ct_ebus_handler *handler) {
    disconnect_addr(bus_name, event, 0, handler);
}


uint32_t event_count(uint32_t bus_name) {
    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return 0;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    return ct_array_size(ebus->events);
}

uint64_t *events(uint32_t bus_name) {
    uint64_t ebus_idx = ct_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return 0;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    return ebus->events;
}

static struct ct_ebus_a0 _api = {
        .create_ebus = create_ebus,
        .broadcast = broadcast,
        .send = send_addr,
        .connect = _connect,
        .connect_addr = _connect_addr,
        .begin_frame = begin_frame,

        .disconnect = disconnect,
        .disconnect_addr = disconnect_addr,

        .event_count = event_count,
        .events = events,
};

struct ct_ebus_a0 *ct_ebus_a0 = &_api;

static void _init(struct ct_api_a0 *api) {

    api->register_api("ct_ebus_a0", &_api);

    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator()
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
