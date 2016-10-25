#ifndef CETECH_STRINGID_H
#define CETECH_STRINGID_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/types.h"

typedef struct {
    u64 id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

stringid64_t stringid64_from_string(const char *str);

#endif //CETECH_STRINGID_H
