#pragma once

#include "common/cvars.h"

namespace cetech {
    namespace cvars {
        CVar rm_build_dir("rm.build_dir", "Path to build data dir.", "./data/build/", CVar::FLAG_CONST);
        CVar rm_source_dir("rm.source_dir", "Path to source data dir.", "./data/src/", CVar::FLAG_CONST);
    }
}