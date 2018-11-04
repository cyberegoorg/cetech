//==============================================================================
// Includes
//==============================================================================


#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/os.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/ebus.h>
#include <celib/cdb.h>
#include <celib/macros.h>
#include <celib/hash.inl>
#include <celib/hashlib.h>

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
    ce_ebus_handler *handler;
};

struct ebus_event_handlers {
    struct ebus_event_handler *handlers;
};

struct ebus_t {
    uint64_t bus_name;

    struct ce_hash_t handler_idx;
    struct ebus_event_handlers *handlers;
    uint8_t *events;
};

static struct _G {
    struct ce_hash_t ebus_idx;
    struct ebus_t *ebus_pool;

    struct ce_alloc *allocator;
} _G;


//==============================================================================
// Api
//==============================================================================


void create_ebus(uint64_t id) {
    uint64_t ebus_idx = ce_array_size(_G.ebus_pool);

    struct ebus_t ebus = {
            .bus_name = id,
    };

    ce_array_push(_G.ebus_pool, ebus, _G.allocator);
    ce_hash_add(&_G.ebus_idx, ebus.bus_name, ebus_idx, _G.allocator);
}


void send_addr(uint64_t bus_name,
               uint64_t addr,
               uint64_t type,
               void *event,
               uint32_t size) {

    uint64_t ebus_idx = ce_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];
    const size_t event_size = sizeof(struct ebus_event_header) + size;

    char ev[event_size];
    *((struct ebus_event_header *) ev) = (struct ebus_event_header) {
            .type = type,
            .size = event_size,
    };
    memcpy(ev + sizeof(struct ebus_event_header), event, size);

    ce_array_push_n(ebus->events,
                    ev,
                    event_size, _G.allocator);

    uint64_t event_type = type;

    uint64_t event_idx = ce_hash_lookup(&ebus->handler_idx, event_type,
                                        UINT64_MAX);
    if (UINT64_MAX == event_idx) {
        return;
    }
    struct ebus_event_handlers *ev_handlers = &ebus->handlers[event_idx];

    const uint32_t handlers_n = ce_array_size(ev_handlers->handlers);
    for (int i = 0; i < handlers_n; ++i) {
        if (ev_handlers->handlers[i].addr &&
            (ev_handlers->handlers[i].addr != addr)) {
            continue;
        }

        ev_handlers->handlers[i].handler(type, event);
    }
}

void send_addr_obj(uint64_t bus_name,
                   uint64_t addr,
                   uint64_t type,
                   uint64_t event) {
    struct ebus_cdb_event ev = {event};
    send_addr(bus_name, addr, type, &ev, sizeof(ev));
}

void broadcast_obj(uint64_t bus_name,
                   uint64_t type,
                   uint64_t event) {
    struct ebus_cdb_event ev = {event};
    send_addr(bus_name, 0, type, &ev, sizeof(ev));
}

void broadcast(uint64_t bus_name,
               uint64_t type,
               void *event,
               uint32_t size) {
    send_addr(bus_name, 0, type, event, size);
}

void begin_frame() {
    uint32_t ebus_n = ce_array_size(_G.ebus_pool);
    for (int i = 0; i < ebus_n; ++i) {
        struct ebus_t *ebus = &_G.ebus_pool[i];
        if (ce_array_any(ebus->events)) {

            struct ebus_event_header *it = ce_ebus_a0->events(ebus->bus_name);
            struct ebus_event_header *end_it = ce_ebus_a0->events_end(
                    ebus->bus_name);

            while (it != end_it) {

                if (it->size != sizeof(struct ebus_event_header)) {
                    struct ebus_cdb_event *obj_event = CE_EBUS_BODY(it);
                    ce_cdb_a0->destroy_object(obj_event->obj);
                }

                it = CE_EBUS_NEXT(it);
            }

            ce_array_resize(ebus->events, 0, _G.allocator);
        }
    }
}

void _connect_addr(uint64_t bus_name,
                   uint64_t event,
                   uint64_t addr,
                   ce_ebus_handler *handler,
                   uint32_t order) {

    uint64_t ebus_idx = ce_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    uint64_t event_idx = ce_hash_lookup(&ebus->handler_idx, event, UINT64_MAX);

    if (UINT64_MAX == event_idx) {
        event_idx = ce_array_size(ebus->handlers);

        struct ebus_event_handlers ev_handlers = {};
        ce_array_push(ebus->handlers, ev_handlers, _G.allocator);

        ce_hash_add(&ebus->handler_idx, event, event_idx, _G.allocator);
    }

    struct ebus_event_handlers *ev_handlers = &ebus->handlers[event_idx];

    struct ebus_event_handler h = {
            .handler = handler,
            .addr = addr,
            .order = order,
    };

    const uint32_t handlers_n = ce_array_size(ev_handlers->handlers);

    if (0 == handlers_n) {
        ce_array_push(ev_handlers->handlers, h, _G.allocator);
        return;
    }

    for (int i = 0; i < handlers_n; ++i) {
        if (ev_handlers->handlers[i].order < order) {
            continue;
        }

        ce_array_insert(ev_handlers->handlers, i, h, _G.allocator);
        return;
    }

    ce_array_push(ev_handlers->handlers, h, _G.allocator);
}

void _connect(uint64_t bus_name,
              uint64_t event,
              ce_ebus_handler *handler,
              uint32_t order) {
    _connect_addr(bus_name, event, 0, handler, order);
}

void disconnect_addr(uint64_t bus_name,
                     uint64_t event,
                     uint64_t addr,
                     ce_ebus_handler *handler) {
    uint64_t ebus_idx = ce_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    uint64_t event_idx = ce_hash_lookup(&ebus->handler_idx, event, UINT64_MAX);

    if (UINT64_MAX == event_idx) {
        return;
    }

    struct ebus_event_handlers *ev_handlers = &ebus->handlers[event_idx];
    const uint32_t handlers_n = ce_array_size(ev_handlers->handlers);

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
        ce_array_pop_back(ev_handlers->handlers);
        return;
    }
}

void disconnect(uint64_t bus_name,
                uint64_t event,
                ce_ebus_handler *handler) {
    disconnect_addr(bus_name, event, 0, handler);
}


uint32_t event_count(uint64_t bus_name) {
    uint64_t ebus_idx = ce_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return 0;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    return ce_array_size(ebus->events);
}

struct ebus_event_header *events(uint64_t bus_name) {
    uint64_t ebus_idx = ce_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return 0;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    return (struct ebus_event_header *) ebus->events;
}

struct ebus_event_header *events_end(uint64_t bus_name) {
    uint64_t ebus_idx = ce_hash_lookup(&_G.ebus_idx, bus_name, 0);

    if (!ebus_idx) {
        return 0;
    }

    struct ebus_t *ebus = &_G.ebus_pool[ebus_idx];

    return (struct ebus_event_header *) (((char *) ebus->events) +
                                         ce_array_size(ebus->events));
}

static struct ce_ebus_a0 _api = {
        .create_ebus = create_ebus,
        .broadcast = broadcast,
        .send = send_addr,
        .broadcast_obj = broadcast_obj,
        .send_obj = send_addr_obj,
        .connect = _connect,
        .connect_addr = _connect_addr,
        .begin_frame = begin_frame,

        .disconnect = disconnect,
        .disconnect_addr = disconnect_addr,

//        .event_count = event_count,
        .events = events,
        .events_end = events_end,
};

struct ce_ebus_a0 *ce_ebus_a0 = &_api;

static void _init(struct ce_api_a0 *api) {

    api->register_api("ce_ebus_a0", &_api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    create_ebus(0);
}

static void _shutdown() {
    _G = (struct _G) {};
}

CE_MODULE_DEF(
        ebus,
        {
            CE_UNUSED(api);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)
