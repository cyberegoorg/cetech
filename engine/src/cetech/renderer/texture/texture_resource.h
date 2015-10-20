#pragma once

#include "celib/string/stringid_types.h"
#include "celib/memory/memory.h"
#include "cetech/os/os.h"
#include "celib/crypto/murmur_hash.inl.h"
#include "cetech/filesystem/file.h"
#include "cetech/resource_compiler/resource_compiler.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "bgfx/bgfx.h"

namespace cetech {
    namespace resource_texture {
        struct Resource {
            const bgfx::Memory* mem;
            bgfx::TextureHandle handle;
        };

        StringId64_t type_hash();

        void compiler(const char* filename, CompilatorAPI& compilator);
        char* loader(FSFile* f, Allocator& a);
        void online(void* data);
        void offline(void* data);
        void unloader(Allocator& a, void* data);

    }
}