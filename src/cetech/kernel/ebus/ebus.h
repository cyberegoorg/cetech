#ifndef CETECH_EBUS_H
#define CETECH_EBUS_H



//==============================================================================
// Includes
//==============================================================================
#include <stdint.h>
#include <cetech/kernel/cdb/cdb.h>

//==============================================================================
// Structs
//==============================================================================

struct ebus_header_t {
    uint64_t type;
    uint64_t size;
};

typedef void (ct_ebus_handler)(struct ct_cdb_obj_t *event);

//==============================================================================
// Api
//==============================================================================

struct ct_ebus_a0 {
    void (*create_ebus)(const char *name,
                        uint32_t id);

    void (*begin_frame)();

    void (*broadcast)(uint32_t bus_name, struct ct_cdb_obj_t* event);

    void (*send)(uint32_t bus_name,
                 uint64_t addr,
                 struct ct_cdb_obj_t* event);

    void (*connect)(uint32_t bus_name,
                    uint64_t event,
                    ct_ebus_handler *handler,
                    uint32_t order);

    void (*connect_addr)(uint32_t bus_name,
                         uint64_t event,
                         uint64_t addr,
                         ct_ebus_handler *handler,
                         uint32_t order);

    void (*disconnect)(uint32_t bus_name,
                       uint64_t event,
                       ct_ebus_handler *handler);

    void (*disconnect_addr)(uint32_t bus_name,
                            uint64_t event,
                            uint64_t addr,
                            ct_ebus_handler *handler);



    uint32_t (*event_count)(uint32_t bus_name);

    struct ct_cdb_obj_t** (*events)(uint32_t bus_name);
};



#endif //CETECH_EBUS_H