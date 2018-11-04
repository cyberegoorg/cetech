#ifndef CE_EBUS_H
#define CE_EBUS_H


#include <stdint.h>
#include <celib/module.inl>

struct ebus_event_header {
    uint64_t type;
    uint64_t size;
};

struct ebus_cdb_event {
    uint64_t obj;
};

#define CE_EBUS_BODY(event) \
    ((void*)(((uint8_t*)event)+sizeof(struct ebus_event_header)))

#define CE_EBUS_NEXT(event) \
    ((struct ebus_event_header*)(((uint8_t*)event)+((event)->size)))

typedef void (ce_ebus_handler)(uint64_t type,
                               void *event);

struct ce_ebus_a0 {
    void (*create_ebus)(uint64_t id);

    void (*begin_frame)();

    void (*send)(uint64_t bus_name,
                 uint64_t addr,
                 uint64_t type,
                 void *event,
                 uint32_t size);

    void (*send_obj)(uint64_t bus_name,
                     uint64_t addr,
                     uint64_t type,
                     uint64_t event);

    void (*broadcast)(uint64_t bus_name,
                      uint64_t type,
                      void *event,
                      uint32_t size);

    void (*broadcast_obj)(uint64_t bus_name,
                          uint64_t type,
                          uint64_t event);

    void (*connect)(uint64_t bus_name,
                    uint64_t event,
                    ce_ebus_handler *handler,
                    uint32_t order);

    void (*connect_addr)(uint64_t bus_name,
                         uint64_t event,
                         uint64_t addr,
                         ce_ebus_handler *handler,
                         uint32_t order);

    void (*disconnect)(uint64_t bus_name,
                       uint64_t event,
                       ce_ebus_handler *handler);

    void (*disconnect_addr)(uint64_t bus_name,
                            uint64_t event,
                            uint64_t addr,
                            ce_ebus_handler *handler);


//    uint32_t (*event_count)(uint64_t bus_name);

    struct ebus_event_header *(*events)(uint64_t bus_name);

    struct ebus_event_header *(*events_end)(uint64_t bus_name);
};

CE_MODULE(ce_ebus_a0);

#endif //CE_EBUS_H