#include "cetech/lua/lua_resource.h"
#include "cetech/lua/lua_enviroment.h"
#include "cetech/lua/lua_stack.h"

#include "celib/string/stringid_types.h"
#include "celib/memory/memory.h"
#include "celib/string/stringid.inl.h"

#include "cetech/os/os.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace cetech {
    namespace resource_lua {
        static StringId64_t _type_hash = stringid64::from_cstring("lua");

        StringId64_t type_hash() {
            return _type_hash;
        }


        void compiler(const char* filename, FSFile* in, FSFile* out) {
            size_t sz_in = in->size();
            char tmp[4096] = {0};
            in->read(tmp, sz_in);
	    
	    
	    // TODO: lua + LuaStack?, inline lua? 
	    lua_State* state = luaL_newstate();
	    luaL_openlibs(state);
            CE_CHECK_PTR(state);
	    LuaStack s(state);
	    s.execute_string(
	      "function compile(what, filename,  strip)\n"
	      " local s, err = loadstring(what, filename)\n"
	      " if s ~= nil then\n"
	      "   return string.dump(s, strip), nil\n"
	      " end\n"
	      " return nil, err\n"
	      "end"
	    );

	    lua_getglobal(state, "compile");
	    s.push_string(tmp);
	    s.push_string(filename);
#if defined(CETECH_DEBUG)
	    s.push_bool(false);
#else
	    s.push_bool(true);
#endif

	    lua_pcall(state, 3, 2, 0);
	    if(lua_isnil(state, 1)) {
	      const char* err = s.to_string(2);
	      log::error("lua_resource.compiler", "%s", err);

	    } else {
	      size_t bc_len = 0;
	      const char* bc = s.to_string(1, &bc_len);

	      Resource r;
	      r.type = 1;
	      r.size = bc_len;

	      out->write(&r, sizeof(Resource));
	      out->write(bc, bc_len);
	    }

	    lua_close(state);
        }

        void* loader (FSFile* f, Allocator& a) {
            const uint64_t f_sz = f->size();

            void* mem = a.allocate(f_sz);
            f->read(mem, f_sz);

            return mem;
        }

        void unloader(Allocator& a, void* data) {
            a.deallocate(data);
        }

        const char* get_source(const Resource* rs) {
            return (const char*)(rs + 1);
        }

    }
}