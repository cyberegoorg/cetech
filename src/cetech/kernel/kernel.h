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

#define CONFIG_COMPILE \
     CE_ID64_0("compile", 0x3c797c340e1e5467ULL)

#define CONFIG_BOOT_PKG \
     CE_ID64_0("core.boot_pkg", 0xd065c28a0c45037eULL)

#define CONFIG_BOOT_ENT \
     CE_ID64_0("core.boot_ent", 0x111eabf289640feeULL)

#define CONFIG_CONTINUE \
     CE_ID64_0("continue", 0xb3326d55cc9374b8ULL)

#define CONFIG_WAIT \
     CE_ID64_0("wait", 0x21778fd44dd2ab6aULL)

#define CONFIG_GAME \
     CE_ID64_0("game", 0x8a426cdbe2604989ULL)

#define CONFIG_DAEMON \
     CE_ID64_0("daemon", 0xc3b953e09c1d1f60ULL)

#define SOURCE_ROOT \
    CE_ID64_0("source", 0x921f1370045bad6eULL)

#define BUILD_ROOT \
    CE_ID64_0("build", 0x4429661936ece1eaULL)

#define CT_INPUT_TASK \
    CE_ID64_0("input_task", 0xba6140c0d9e00706ULL)

#define KERNEL_TASK_INTERFACE \
    CE_ID64_0("ct_kernel_task_i0", 0xc47eec37e164c0a7ULL)

typedef void (*ce_kernel_taks_update_t)(float dt);
typedef void (*ce_kernel_taks_init_t)();
typedef void (*ce_kernel_taks_shutdown_t)();

typedef struct ct_kernel_task_i0 {
    uint64_t (*name)();

    void (*update)(float dt);
    uint64_t* (*update_before)(uint64_t* n);
    uint64_t* (*update_after)(uint64_t* n);

    void (*init)();
    void (*shutdown)();
    uint64_t* (*init_before)(uint64_t* n);
    uint64_t* (*init_after)(uint64_t* n);
} ct_kernel_task_i0;

struct ct_kernel_a0 {
    void (*quit)();
};


CE_MODULE(ct_kernel_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_KERNEL_H
