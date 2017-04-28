#ifndef CELIB_STRINGID_H
#define CELIB_STRINGID_H

//==============================================================================
// Includes
//==============================================================================

#include "string.h"
#include "types.h"
#include "hash.h"

typedef struct {
    uint64_t id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

stringid64_t stringid64_from_string(const char *str);

#endif //CELIB_STRINGID_H
