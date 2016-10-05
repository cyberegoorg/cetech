#ifndef CETECH_RESOURCE_COMPILER_H
#define CETECH_RESOURCE_COMPILER_H

//==============================================================================
// Include
//==============================================================================

#include <celib/stringid/types.h>
#include <celib/os/vio_types.h>


//==============================================================================
// Structs and typedefs
//==============================================================================

struct compilator_api {
    void (*add_dependency)(const char *who_filname,
                           const char *depend_on_filename);
};

typedef int (*resource_compilator_t)(
        const char *filename,
        struct vio *source_vio,
        struct vio *build_vio,
        struct compilator_api *compilator_api);


//==============================================================================
// Interface
//==============================================================================

int resource_compiler_init(int stage);

void resource_compiler_shutdown();

void resource_compiler_register(stringid64_t type,
                                resource_compilator_t compilator);

void resource_compiler_compile_all();

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   stringid64_t type,
                                   stringid64_t name);

const char *resource_compiler_get_source_dir();

const char *resource_compiler_get_core_dir();

int resource_compiler_get_build_dir(char *build_dir,
                                    size_t max_len,
                                    const char *platform);

int resource_compiler_get_tmp_dir(char *tmp_dir,
                                  size_t max_len,
                                  const char *platform);

int resource_compiler_external_join(char *output,
                                    u32 max_len,
                                    const char *name);

void resource_compiler_create_build_dir();

#endif //CETECH_RESOURCE_COMPILER_H
