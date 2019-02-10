#ifndef CETECH_SELECTED_OBJECT_H
#define CETECH_SELECTED_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <celib/celib_types.h>

#define CT_SELECTED_OBJECT_API \
    CE_ID64_0("ct_selected_object_a0", 0x9ea58b4cfb24327cULL)


struct ct_selected_object_a0 {
    void (*set_selected_object)(uint64_t context,
                                uint64_t obj);

    uint64_t (*selected_object)(uint64_t context);

    void (*set_previous)(uint64_t context);
    void (*set_next)(uint64_t context);

    bool (*has_previous)(uint64_t context);
    bool (*has_next)(uint64_t context);
};

CE_MODULE(ct_selected_object_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_SELECTED_OBJECT_H
