#ifndef CETECH_RESOURCE_COMPILER_H
#define CETECH_RESOURCE_COMPILER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CT_RESOURCE_COMPILER_API \
    CE_ID64_0("ct_resource_compiler_a0", 0xc084f9ae737212dcULL)

#define CONFIG_SRC \
     CE_ID64_0("src", 0x1cdb3620898c588eULL)

#define CONFIG_CORE \
     CE_ID64_0("core", 0x6da99857e9315560ULL)

#define CONFIG_MODULE_DIR \
     CE_ID64_0("module_dir", 0xa96daa49986032f4ULL)

#define CONFIG_EXTERNAL \
     CE_ID64_0("external", 0x9fb8bb487a62dc4fULL)


struct ce_vio;
struct ce_alloc_t0;
struct ct_resource_id;

struct ct_resource_compiler_a0 {
    void (*compile_all)();

    char *(*get_tmp_dir)(struct ce_alloc_t0 *a,
                         const char *platform);

    char *(*external_join)(struct ce_alloc_t0 *a,
                           const char *name);
};

CE_MODULE(ct_resource_compiler_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_RESOURCE_COMPILER_H
