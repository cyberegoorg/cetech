#pragma once

#include <cstring>
#include <cstdlib>

#include "platform/defines.h"
#include "common/stringid_types.h"

namespace cetech {
    struct CVar {
        char name[512];
        char desc[1024];
        StringId64_t hash;

        CVar* _next;

        enum CVarType {
            CVAR_INT = 1,
            CVAR_FLOAT,
            CVAR_STR,
        } type;

        enum CVarFlags {
            FLAG_NONE = 0,
            FLAG_CONST = 1,
        };
        uint32_t flags;

        union {
            char* value_str;
            float value_f;
            int value_i;
        };

        union {
            float f_max;
            int i_max;
        };

        union {
            float f_min;
            int i_min;
        };

        CE_INLINE CVar(const char* name, const char* desc, CVarType type, uint32_t flags);

        CE_INLINE CVar(const char* name,
                       const char* desc,
                       float value,
                       uint32_t flags = FLAG_NONE,
                       float min = 0.0f,
                       float max = 0.0f);

        CE_INLINE CVar(const char* name,
                       const char* desc,
                       int value,
                       uint32_t flags = FLAG_NONE,
                       int min = 0,
                       int max = 0);

        CE_INLINE CVar(const char* name, const char* desc, const char* value, uint32_t flags = FLAG_NONE);
    };
}