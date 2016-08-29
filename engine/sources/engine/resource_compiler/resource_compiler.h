#ifndef CETECH_RESOURCE_COMPILER_H
#define CETECH_RESOURCE_COMPILER_H

//==============================================================================
// Private
//==============================================================================

#include <celib/stringid/types.h>
#include <celib/os/vio_types.h>


//==============================================================================
// Structs and typedefs
//==============================================================================

struct compilator_api {
    void (*add_dependency)(const char *who_filname, const char *depend_on_filename);
};

typedef int (*resource_compilator_t)(const char *filename,
                                     struct vio *source_vio,
                                     struct vio *build_vio,
                                     struct compilator_api *compilator_api);


//==============================================================================
// Interface
//==============================================================================

int resource_compiler_init();

void resource_compiler_shutdown();

void resource_compiler_register(stringid64_t type, resource_compilator_t compilator);

void resource_compiler_compile_all();

#endif //CETECH_RESOURCE_COMPILER_H
