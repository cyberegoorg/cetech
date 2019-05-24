#ifndef CETECH_DOCK_H
#define CETECH_DOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>


#define CT_DOCK_API \
    CE_ID64_0("ct_dock_a0", 0x90cc2a5b4273931aULL)

#define DOCK_INTERFACE_NAME \
    "ct_dock_i0"

#define DOCK_INTERFACE \
    CE_ID64_0("ct_dock_i0", 0x3a3181c12a9e109fULL)

#define DOCK_TYPE\
    CE_ID64_0("dock", 0x3949d96def384d3cULL)

#define PROP_DOCK_VISIBLE\
    CE_ID64_0("visible", 0x24b77991b5d39298ULL)

#define PROP_DOCK_flags\
    CE_ID64_0("flags", 0x6def9a496b03ba91ULL)

#define PROP_DOCK_CONTEXT\
    CE_ID64_0("context", 0x1349eedc2838510ULL)

#define PROP_DOCK_TYPE \
    CE_ID64_0("dock_type", 0x5cea667e25e92f1dULL)

#define PROP_DOCK_DATA \
    CE_ID64_0("dock_data", 0x944132def1f8c765ULL)

#define PROP_DOCK_SELECTED_OBJ\
    CE_ID64_0("selected_object", 0x7cc97924b2b963b1ULL)


#define DOCKS_LAYOUT_TYPE \
    CE_ID64_0("docks_layout", 0x7336aa61594d9ff9ULL)

#define DOCK_LAYOUT_TYPE \
    CE_ID64_0("dock_layout", 0x4e4af47b58821c36ULL)

#define DOCK_DOCKS \
    CE_ID64_0("docks", 0xf9937f250674149ULL)

#define DOCK_INDEX \
    CE_ID64_0("index", 0xb03b8bced9422c44ULL)

#define DOCK_LABEL \
    CE_ID64_0("label", 0x29f0c34fda4b3acfULL)

#define DOCK_LOCATION \
    CE_ID64_0("location", 0x104249e36e86d778ULL)

#define DOCK_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)

#define DOCK_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)

#define DOCK_SIZE_X \
    CE_ID64_0("size_x", 0xacc8adbd22d2416bULL)

#define DOCK_SIZE_Y \
    CE_ID64_0("size_y", 0x1211758e2a991897ULL)

#define DOCK_STATUS \
    CE_ID64_0("status", 0xcdb5c90212e5367fULL)

#define DOCK_ACTIVE \
    CE_ID64_0("active", 0xb19ad128fab64ca4ULL)

#define DOCK_OPENED \
    CE_ID64_0("opened", 0xa33c08a6d3dc9422ULL)

#define DOCK_PREV \
    CE_ID64_0("prev", 0x29ec8f894c44aa64ULL)

#define DOCK_NEXT \
    CE_ID64_0("next", 0x278440c8fcb56970ULL)

#define DOCK_CHILD0 \
    CE_ID64_0("child0", 0x689ee2e497b819b4ULL)

#define DOCK_CHILD1 \
    CE_ID64_0("child1", 0xdec55fcd7f7396daULL)

#define DOCK_PARENT \
    CE_ID64_0("parent", 0xc46a4b4368e0f801ULL)

typedef struct ct_dock_i0 {
    uint64_t (*cdb_type)();

    uint64_t (*dock_flags)();

    uint64_t (*open)(uint64_t dock);

    void (*close)(uint64_t dock, uint64_t context);

    const char *(*display_title)(uint64_t dock);

    const char *(*name)(uint64_t dock);

    void (*draw_ui)(uint64_t dock);

    void (*draw_menu)(uint64_t dock);

    void (*draw_main_menu)(uint64_t dock);
} ct_dock_i0;


struct ct_dock_a0 {
    uint64_t (*create_dock)(uint64_t type,
                            bool visible);

    bool (*context_btn)(uint64_t dock);

    void (*draw_all)();

    void (*draw_menu)();
};

CE_MODULE(ct_dock_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_DOCK_H
