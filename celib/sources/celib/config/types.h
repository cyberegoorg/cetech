#ifndef CELIB_CONFIG_TYPES_H
#define CELIB_CONFIG_TYPES_H

typedef struct {
    u64 idx;
} cvar_t;

enum cvar_type {
    CV_NONE = 0,
    CV_FLOAT,
    CV_INT,
    CV_STRING
};

#endif //CELIB_CONFIG_TYPES_H
