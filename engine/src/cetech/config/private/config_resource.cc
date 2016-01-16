#include "cetech/renderer/texture_resource.h"

#include "celib/container/array.inl.h"
#include "celib/string/types.h"
#include "celib/memory/memory.h"
#include "celib/macros.h"
#include "celib/string/stringid.inl.h"

#include "yaml/yaml.h"

#include "cetech/cvars/cvars.h"

namespace cetech {
    namespace resource_config {
        static const StringId64_t _type_hash = stringid64::from_cstring("config");

        StringId64_t type_hash() {
            return _type_hash;
        }   


        void compile(const char* filename,
                     CompilatorAPI& compilator) {
            CE_UNUSED(filename);
            
            const size_t config_size =  compilator.resource_file_size();
            char buffer[config_size];
            compilator.read_resource_file(buffer);

            compilator.write_to_build(buffer, config_size);   
        }

        char* loader (FSFile& f,
                      Allocator& a) {
            error::ErrorScope es("loading config", nullptr);
            
            const size_t config_size =  f.size();
            char buffer[config_size];
            
            f.read(buffer, config_size);
            
            cvar::load_from_yaml(buffer, config_size);
            
            return (char*)1; // TODO =(
        }

        void online(void* data) {
        }

        void offline(void* data) {
        }

        void unloader(Allocator& a,
                      void* data) {
            //a.deallocate(data);
        }
    }
}
