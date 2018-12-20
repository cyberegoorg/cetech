#ifndef CETECH_RESOURCE_COMPILER_H
#define CETECH_RESOURCE_COMPILER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <celib/module.inl>

#define CONFIG_SRC \
     CE_ID64_0("src", 0x1cdb3620898c588eULL)

#define CONFIG_CORE \
     CE_ID64_0("core", 0x6da99857e9315560ULL)

#define CONFIG_MODULE_DIR \
     CE_ID64_0("module_dir", 0xa96daa49986032f4ULL)

#define CONFIG_EXTERNAL \
     CE_ID64_0("external", 0x9fb8bb487a62dc4fULL)


struct ce_vio;
struct ce_alloc;
struct ct_resource_id;

struct ct_resource_compiler_a0 {
    void (*compile_all)();

    void (*compile_and_reload)(uint64_t name);

    char *(*get_tmp_dir)(struct ce_alloc *a,
                         const char *platform);

    char *(*external_join)(struct ce_alloc *a,
                           const char *name);

//    bool (*type_name_from_filename)(const char *fullname,
//                                    struct ct_resource_id *resource_id,
//                                    char *short_name);
};

CE_MODULE(ct_resource_compiler_a0);

#endif //CETECH_RESOURCE_COMPILER_H
