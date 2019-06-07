#ifndef CETECH_PARENT_H
#define CETECH_PARENT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ct_entity_t0 ct_entity_t0;

#define CT_PARENT_SYSTEM \
    CE_ID64_0("parent_system", 0xee0a3c196a1183d0ULL)

#define CT_PARENT_COMPONENT \
    CE_ID64_0("parent", 0xc46a4b4368e0f801ULL)

#define CT_PREVIOUS_PARENT_COMPONENT \
    CE_ID64_0("previous_parent", 0xa8164f5a3848444bULL)

#define CT_CHILD_COMPONENT \
    CE_ID64_0("child", 0x88d0eb1725682035ULL)

typedef struct ct_parent_c {
    ct_entity_t0 parent;
} ct_parent_c;

typedef struct ct_previous_parent_c {
    ct_entity_t0 parent;
} ct_previous_parent_c;

typedef struct ct_child_c {
    ct_entity_t0 *child;
} ct_child_c;

#ifdef __cplusplus
};
#endif

#endif //CETECH_PARENT_H