#include "cetech/cvars/cvars.h"

#define xstr(s) str(s)
#define str(s) #s

namespace cetech {
    namespace cvars {
        CVar rm_build_dir("rm.build_dir", "Path to build data dir.", "./data/build/", CVar::FLAG_CONST);
        CVar rm_source_dir("rm.source_dir", "Path to source data dir.", "./data/src/", CVar::FLAG_CONST);

        CVar boot_pkg("boot.pkg", "Boot package", "boot", CVar::FLAG_CONST);
        CVar boot_script("boot.script", "Boot script", "lua/boot", CVar::FLAG_CONST);

        CVar console_server_port("console_server.port", "Console server port.", 2222, CVar::FLAG_CONST);
	
	CVar compiler_platform("compiler.platform", "Compiler platform", xstr(CETECH_PLATFORM), CVar::FLAG_CONST);
	CVar compiler_core_path("compiler.core_path", "Compiler core path", "./core/", CVar::FLAG_CONST);
    }
}