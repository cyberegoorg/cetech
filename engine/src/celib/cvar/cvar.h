#pragma once

#include <cstring>
#include <cstdlib>
#include <iostream>

#include "yaml/yaml.h"

#include "celib/errors/errors.h"
#include "celib/cvar/cvar_types.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "celib/string/types.h"

#include "celib/log/log.h"

namespace cetech {
    namespace cvar {
        void force_set(CVar& cv,
                       float value);
        
        void force_set(CVar& cv,
                       int value);

        void force_set(CVar& cv,
                       const char* str);
        
        void set(CVar& cv,
                           float value);
        void set(CVar& cv,
                 int value);
        
        void set(CVar& cv,
                 const char* str);

        CVar* find(const char* name);

        void load_from_yaml(const char* yaml_str,
                                      const size_t len);

        void dump_all();
    }
}
