#ifndef CE_EBUS_H
#define CE_EBUS_H


#include <stdint.h>
#include <celib/cdb.h>

typedef void (ce_ebus_handler)(uint64_t event);

struct ce_ebus_a0 {
    void (*create_ebus)(const char *name,
                        uint64_t id);

    void (*begin_frame)();

    void (*broadcast)(uint64_t bus_name,
                      uint64_t event);

    void (*send)(uint64_t bus_name,
                 uint64_t addr,
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


    uint32_t (*event_count)(uint64_t bus_name);

    uint64_t *(*events)(uint64_t bus_name);
};

CE_MODULE(ce_ebus_a0);

#endif //CE_EBUS_H