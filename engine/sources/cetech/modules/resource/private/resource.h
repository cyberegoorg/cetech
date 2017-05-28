#ifndef CETECH_RESOURCE_H
#define CETECH_RESOURCE_H

#include <cetech/core/hash.h>
#include <cetech/modules/resource/resource.h>

void resource_set_autoload(int enable);

void resource_register_type(uint64_t type,
                            resource_callbacks_t callbacks);

void resource_load(void **loaded_data,
                   uint64_t type,
                   uint64_t *names,
                   size_t count,
                   int force);

void resource_add_loaded(uint64_t type,
                         uint64_t *names,
                         void **resource_data,
                         size_t count);

void resource_load_now(uint64_t type,
                       uint64_t *names,
                       size_t count);

void resource_unload(uint64_t type,
                     uint64_t *names,
                     size_t count);

void resource_reload(uint64_t type,
                     uint64_t *names,
                     size_t count);

void resource_reload_all();

int resource_can_get(uint64_t type,
                     uint64_t names);

int resource_can_get_all(uint64_t type,
                         uint64_t *names,
                         size_t count);

void *resource_get(uint64_t type,
                   uint64_t names);

int resource_type_name_string(char *str,
                              size_t max_len,
                              uint64_t type,
                              uint64_t name);

#ifdef CETECH_CAN_COMPILE

void resource_compiler_register(uint64_t type,
                                resource_compilator_t compilator);

void resource_compiler_compile_all();

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   uint64_t type,
                                   uint64_t name);

const char *resource_compiler_get_source_dir();


int resource_compiler_get_tmp_dir(char *tmp_dir,
                                  size_t max_len,
                                  const char *platform);

int resource_compiler_external_join(char *output,
                                    uint32_t max_len,
                                    const char *name);

void resource_compiler_create_build_dir();

const char *resource_compiler_get_core_dir();

#endif


int resource_compiler_get_build_dir(char *build_dir,
                                    size_t max_len,
                                    const char *platform);

void package_load(uint64_t name);

void package_unload(uint64_t name);

int package_is_loaded(uint64_t name);

void package_flush(uint64_t name);

#endif //CETECH_RESOURCE_H
