#ifndef CELIB_STRINGID_H
#define CELIB_STRINGID_H

//==============================================================================
// Includes
//==============================================================================

#include "string.h"
#include "types.h"
#include "hash.h"

typedef struct {
    u64 id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

stringid64_t stringid64_from_string(const char *str);

#endif //CELIB_STRINGID_H
