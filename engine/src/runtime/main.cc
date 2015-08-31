#include "common/memory/memory.h"
#include "common/math/math_types.h"
#include "common/math/math.h"
#include "common/math/quat.h"
#include "common/math/vector2.h"
#include "common/math/matrix33.h"
#include "common/math/matrix44.h"
#include "common/math/simd/simd.h"
#include "common/types.h"
#include "common/log.h"
#include "common/container/array.h"
#include "common/container/hash.h"
#include "common/container/queue.h"
#include "common/ecs/entitymanager.h"
#include "runtime/runtime.h"

#include "common/resource/resource_manager.h"
#include "common/resource/package_manager.h"


#include "resources/package.h"

#include "common/stringid_types.h"

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace cetech;
using namespace rapidjson;

namespace cetech {
    struct CVar;
    static CVar *_head = nullptr;
    
    struct CVar {        
        char name[512];
        char desc[1024];
        StringId64_t hash;
        
        CVar *_next;
        
        enum CVarType {
            CVAR_INT = 1,
            CVAR_UINT,
            CVAR_FLOAT,
            CVAR_STR,
        } type;
        
        union {
            const char* str;
            float f;
            int i;
        };
        
        union {
            float f_max;
            int i_max;
        };
        
        union {
            float f_min;
            int i_min;
        };
        
        CVar(const char* name, const char *desc, CVarType type): _next(nullptr), type(type) {
            strncpy(this->name, name, 512);
            strncpy(this->desc, desc, 1024);
            
            hash = murmur_hash_64(name, strlen(name), 22);
            
            f_max = 0.0f;
            f_min = 0.0f;
            
            if( _head == nullptr ) {
                _head = this;
            } else {
                _next = _head;
                _head = this;
            }
        }
        
        CVar(const char* name, const char *desc, float value, float min = 0.0f, float max = 0.0f): CVar(name, desc, CVAR_FLOAT) {
            f_min = min;
            f_max = max;
            f = value;
        }
    };
    
    namespace cvar {
        void dump_all() {
            CVar *it = _head;
            
            uint32_t pos = 0;
            
            while(it != nullptr) {
                
                switch(it->type) {
                    case CVar::CVAR_INT:
                        log::info("CVar.dump", "[%u] name: %s  value: %i  desc: %s", pos, it->name, it->i, it->desc);
                        break;

                    case CVar::CVAR_FLOAT:
                        log::info("CVar.dump", "[%u] name: %s  value: %f  desc: %s", pos, it->name, it->f, it->desc);
                        break;
                        
                    case CVar::CVAR_STR:
                        log::info("CVar.dump", "[%u] name: %s  value: \"%s\"  desc: %s", pos, it->name, it->str, it->desc);
                        break;
                }
                
                ++pos;
                it = it->_next;
            }
        }
    }
}

CVar cvar1("name", "desc", 1.0f);
CVar cvar2("name1", "desc", 2.0f);
CVar cvar3("name2", "desc", 3.0f);

void frame_start() {
    runtime::frame_start();
}

void frame_end() {
    runtime::frame_end();
}

void frame_body() {
    if (runtime::mouse::button_state(runtime::mouse::button_index("left"))) {
        Vector2 m = runtime::mouse::axis();
        printf("%f, %f\n", m.x, m.y);
    }
}

void run() {
    while (1) {
        frame_start();
        frame_body();
        frame_end();
    }
}

void init() {
    memory_globals::init();
    runtime::init();
    resource_manager_globals::init();
    package_manager_globals::init();

    uint64_t type_h = murmur_hash_64("package", strlen("package"), 22);
    uint64_t name_h = murmur_hash_64("main", strlen("main"), 22);

    resource_manager::register_unloader(type_h, &resource_package::unloader);
    resource_manager::register_loader(type_h, &resource_package::loader);
    resource_manager::register_compiler(type_h, &resource_package::compiler);

    resource_manager::compile("main.package");
    resource_manager::load(type_h, name_h);

    package_manager::load(name_h);

    cvar::dump_all();
    
    //     Window w = runtime::window::make_window(
    //         "aaa",
    //         runtime::window::WINDOWPOS_CENTERED, runtime::window::WINDOWPOS_CENTERED,
    //         800, 600,
    //         runtime::window::WINDOW_NOFLAG
    //         );

    Matrix44 m1 = matrix44::IDENTITY;

    float det = matrix44::determinant(m1);

    printf("det: %f\n", det);
}

void shutdown() {
    runtime::shutdown();
    memory_globals::shutdown();
    resource_manager_globals::shutdown();
    package_manager_globals::shutdown();
}

int main(int argc, char** argv) {
    init();
    run();
    shutdown();

    return 0; // TODO: error check
}
