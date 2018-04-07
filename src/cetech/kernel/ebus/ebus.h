#ifndef CETECH_EBUS_H
#define CETECH_EBUS_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

//==============================================================================
// Structs
//==============================================================================

struct ebus_header_t {
    uint64_t type;
    uint64_t size;
};

typedef void (ct_ebus_handler)(uint32_t bus_name,
                               void *event);

//==============================================================================
// Api
//==============================================================================

struct ct_ebus_a0 {
    void (*create_ebus)(const char *name,
                        uint32_t id);

    void (*begin_frame)();

    void (*broadcast)(uint32_t bus_name,
                      uint64_t event_type,
                      void *event,
                      uint64_t event_size);

    void (*send)(uint32_t bus_name,
                 uint64_t event_type,
                 uint64_t addr,
                 void *event,
                 uint64_t event_size);

    void (*connect)(uint32_t bus_name,
                    uint64_t event,
                    ct_ebus_handler *handler,
                    uint32_t order);

    void (*connect_addr)(uint32_t bus_name,
                         uint64_t event,
                         uint64_t addr,
                         ct_ebus_handler *handler,
                         uint32_t order);

    void *(*first_event)(uint32_t bus_name);

    void *(*next_event)(uint32_t bus_name,
                        void *event);

    struct ebus_header_t *(*event_header)(void *event);
};


#ifdef __cplusplus
}
#endif

#endif //CETECH_EBUS_H