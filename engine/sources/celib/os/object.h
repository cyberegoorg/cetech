#ifndef CETECH_OBJECT_H
#define CETECH_OBJECT_H


void *os_load_object(const char *path);

void os_unload_object(void *so);

void *os_load_function(void *so,
                       void *name);

#endif //CETECH_OBJECT_H
