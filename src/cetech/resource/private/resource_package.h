#ifndef CETECH_PACKAGE_H
#define CETECH_PACKAGE_H

#include "cetech/macros.h"

struct package_resource {
    uint32_t type_count;
    uint32_t name_count_offset;
    uint32_t type_offset;
    uint32_t name_offset;
    uint32_t offset_offset;
};

#define package_name_count(resource_ptr) ((uint32_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->name_count_offset)))
#define package_offset(resource_ptr) ((uint32_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->offset_offset)))
#define package_type(resource_ptr) ((uint64_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->type_offset)))
#define package_name(resource_ptr) ((uint64_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->name_offset)))

#endif //CETECH_PACKAGE_H
