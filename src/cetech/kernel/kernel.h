#ifndef CETECH_KERNEL_H
#define CETECH_KERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#define CT_KERNEL_API \
    CE_ID64_0("ct_kernel_a0", 0xdb873264c27eedf3ULL)

#define CONFIG_PLATFORM \
     CE_ID64_0("kernel.platform", 0xa2e788cf39b7339bULL)

#define CONFIG_NATIVE_PLATFORM \
     CE_ID64_0("kernel.native_platform", 0xede1e0cf393fc17eULL)

#define CONFIG_BOOT_ENT \
     CE_ID64_0("core.boot_ent", 0x111eabf289640feeULL)

#define CONFIG_GAME \
     CE_ID64_0("game", 0x8a426cdbe2604989ULL)

#define SOURCE_ROOT \
    CE_ID64_0("source", 0x921f1370045bad6eULL)

#define BUILD_ROOT \
    CE_ID64_0("build", 0x4429661936ece1eaULL)

#define CT_INPUT_TASK \
    CE_ID64_0("input_task", 0xba6140c0d9e00706ULL)

#define CT_KERNEL_TASK_I \
    CE_ID64_0("ct_kernel_task_i0", 0xc47eec37e164c0a7ULL)

#define _KERNEL_LIST(...) (uint64_t[]){__VA_ARGS__}
#define _KERNEL_LIST_SIZE(list) (sizeof(list) / sizeof(list[0]))

#define CT_KERNEL_BEFORE(...) \
    ((ce_ptr_pair_t0){.ptr=&_KERNEL_LIST(__VA_ARGS__),.len=_KERNEL_LIST_SIZE(_KERNEL_LIST(__VA_ARGS__))})

#define CT_KERNEL_AFTER(...) \
    ((ce_ptr_pair_t0){.ptr=&_KERNEL_LIST(__VA_ARGS__),.len=_KERNEL_LIST_SIZE(_KERNEL_LIST(__VA_ARGS__))})

typedef void (*ce_kernel_taks_update_t)(float dt);

typedef void (*ce_kernel_taks_init_t)();

typedef void (*ce_kernel_taks_shutdown_t)();

typedef struct ct_kernel_task_i0 {
    uint64_t name;

    ce_ptr_pair_t0 update_before;
    ce_ptr_pair_t0 update_after;

    ce_ptr_pair_t0 init_before;
    ce_ptr_pair_t0 init_after;

    void (*update)(float dt);

    void (*init)();

    void (*shutdown)();
} ct_kernel_task_i0;

struct ct_kernel_a0 {
    void (*quit)();
};


CE_MODULE(ct_kernel_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_KERNEL_H
