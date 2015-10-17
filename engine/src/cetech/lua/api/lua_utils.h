#include "celib/macros.h"

#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

#include "cetech/os/os.h"
#include "celib/string/stringid.inl.h"
#include "cetech/application/application.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

namespace cetech {
    namespace lua_utils {
        static const char* module_name = "Utils";

        void load_libs(LuaEnviroment& env) {
            CE_UNUSED(env);
        }
    }
}