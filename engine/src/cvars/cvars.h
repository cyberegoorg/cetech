#pragma once

#include "common/cvar/cvar.h"

namespace cetech {
    namespace cvars {
        /* Resource manager */
        extern CVar rm_build_dir;
        extern CVar rm_source_dir;

        extern CVar boot_pkg;
        extern CVar boot_script;

        extern CVar console_server_port;
	
	extern CVar compiler_platform;
	extern CVar compiler_core_path;
    }
}