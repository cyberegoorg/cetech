#pragma once

#include "common/cvar.h"

namespace cetech {
    namespace cvars {
        static CVar rm_build_dir("rm.build_dir", "Path to build data dir.", "./data/build/", CVar::FLAG_CONST);
        static CVar rm_source_dir("rm.source_dir", "Path to source data dir.", "./data/src/", CVar::FLAG_CONST);
    }
}