#ifndef CETECH_EBUS_H
#define CETECH_EBUS_H



//==============================================================================
// Includes
//==============================================================================
#include <stdint.h>
#include <corelib/cdb.h>

//==============================================================================
// Structs
//==============================================================================

struct ebus_header_t {
    uint64_t type;
    uint64_t size;
};

typedef void (ct_ebus_handler)(uint64_t event);

//==============================================================================
// Api
//==============================================================================

struct ct_ebus_a0 {
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
                    ct_ebus_handler *handler,
                    uint32_t order);

    void (*connect_addr)(uint64_t bus_name,
                         uint64_t event,
                         uint64_t addr,
                         ct_ebus_handler *handler,
                         uint32_t order);

    void (*disconnect)(uint64_t bus_name,
                       uint64_t event,
                       ct_ebus_handler *handler);

    void (*disconnect_addr)(uint64_t bus_name,
                            uint64_t event,
                            uint64_t addr,
                            ct_ebus_handler *handler);


    uint32_t (*event_count)(uint64_t bus_name);

    uint64_t *(*events)(uint64_t bus_name);
};

CT_MODULE(ct_ebus_a0);

#endif //CETECH_EBUS_H