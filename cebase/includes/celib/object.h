#ifndef CELIB_OBJECT_H
#define CELIB_OBJECT_H

void *cel_load_object(const char *path);

void cel_unload_object(void *so);

void *cel_load_function(void *so,
                        void *name);

#endif //CELIB_OBJECT_H
