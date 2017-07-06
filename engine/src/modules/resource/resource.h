#ifndef CETECH_RESOURCE_INTERNAL_H
#define CETECH_RESOURCE_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cetech/kernel/hash.h>
#include <cetech/modules/resource.h>

#ifdef CETECH_CAN_COMPILE

void resource_compiler_register(uint64_t type,
                                ct_resource_compilator_t compilator);

void resource_compiler_compile_all();

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   uint64_t type,
                                   uint64_t name);

const char *resource_compiler_get_source_dir();


char *resource_compiler_get_tmp_dir(ct_allocator *alocator,
                                    const char *platform);

char *resource_compiler_external_join(ct_allocator *alocator,
                                      const char *name);

void resource_compiler_create_build_dir(struct ct_config_a0 config,
                                        struct ct_app_a0 app);

const char *resource_compiler_get_core_dir();

#endif


char *resource_compiler_get_build_dir(ct_allocator *a,
                                      const char *platform);

#ifdef __cplusplus
}
#endif

int package_init(struct ct_api_a0 *api);

void package_shutdown();

void package_load(uint64_t name);

void package_load(uint64_t name);

void package_unload(uint64_t name);

int package_is_loaded(uint64_t name);

void package_flush(uint64_t name);

#endif //CETECH_RESOURCE_INTERNAL_H
