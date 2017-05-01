#ifndef CETECH_STRINGID_H
#define CETECH_STRINGID_H

//==============================================================================
// Includes
//==============================================================================

typedef struct stringid64_s {
    uint64_t id;
} stringid64_t;

//==============================================================================
// Interface
//==============================================================================

stringid64_t stringid64_from_string(const char *str);

#endif //CETECH_STRINGID_H
