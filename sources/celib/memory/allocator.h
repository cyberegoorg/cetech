/*******************************************************************************
**** Abstract allocator module
*******************************************************************************/

#ifndef CETECH_ALLOCATOR_H
#define CETECH_ALLOCATOR_H


/*******************************************************************************
**** Includes
*******************************************************************************/

#include <stddef.h>


/*******************************************************************************
**** Typedef
*******************************************************************************/
typedef void *Alloc_t;


/*******************************************************************************
**** Macros
*******************************************************************************/

#define ALLOCATOR_CREATE_SCOPED(vname, name)                                    \
    Alloc_t vname __attribute__((cleanup(name##_destructor))) = name##_create() \

#define ALLOCATOR_CREATE(vname, name)                                           \
    Alloc_t vname = name##_create()                                             \

#define ALLOCATOR_DESTROY(vname, name)                                          \
    name##_destroy(vname)                                                       \

/*******************************************************************************
**** Interface
*******************************************************************************/

void *alloc_alloc(Alloc_t allocator, size_t size);
void alloc_free(Alloc_t allocator, void *ptr);
void alloc_destroy(Alloc_t allocator);

#endif //CETECH_ALLOCATOR_H
