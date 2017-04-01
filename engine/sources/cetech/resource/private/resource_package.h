#ifndef CETECH_PACKAGE_H
#define CETECH_PACKAGE_H

#include <celib/types.h>

struct package_resource {
    u32 type_count;
    u32 name_count_offset;
    u32 type_offset;
    u32 name_offset;
    u32 offset_offset;
};

#define package_name_count(resource_ptr) ((u32*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->name_count_offset)))
#define package_offset(resource_ptr) ((u32*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->offset_offset)))
#define package_type(resource_ptr) ((stringid64_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->type_offset)))
#define package_name(resource_ptr) ((stringid64_t*)((void*)(((char*)(resource_ptr)) + (resource_ptr)->name_offset)))

#endif //CETECH_PACKAGE_H
