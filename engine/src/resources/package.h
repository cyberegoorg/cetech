#pragma once

#include "common/memory/memory.h"
#include "runtime/runtime.h"
#include "common/murmur_hash.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace cetech {
    namespace resource_package {
        struct Header {
            uint64_t count;
        };
        
        struct Item{
            uint64_t type;
            uint64_t name;
        };
        
        static void compiler(File& in, File& out);
        static void* loader(File& f, Allocator& a);
    }

    namespace resource_package {
        void compiler(File& in, File& out) {
            size_t sz_in = runtime::file::size(in);
            
            char tmp[4096] = {0};
            runtime::file::read(in, tmp, sz_in, 4096);
            
            rapidjson::Document document;
            document.Parse(tmp);
            
            Header header = {0};
            Item item = {0};
            
            for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
                const rapidjson::Value& ar = itr->value;
                CE_ASSERT(ar.IsArray());
                for (rapidjson::SizeType i = 0; i < ar.Size(); ++i) {
                    ++header.count;
                }
            }
            
             
            runtime::file::write(out, &header, sizeof(Header), 1);
            
            uint64_t type_h = 0;
            uint64_t name_h = 0;
            for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr) {
                item.type = murmur_hash_64(itr->name.GetString(), strlen(itr->name.GetString()), 22);
                
                const rapidjson::Value& ar = itr->value;
                
                CE_ASSERT(ar.IsArray());
                for (rapidjson::SizeType i = 0; i < ar.Size(); ++i) {
                    const rapidjson::Value &v = ar[i];
                    CE_ASSERT(v.IsString());
                    
                    item.name = murmur_hash_64(v.GetString(), strlen(v.GetString()), 22);

                    runtime::file::write(out, &item, sizeof(Item), 1);
                }
            }
        }
        
        void* loader (File& f, Allocator& a) {
            const uint64_t f_sz = runtime::file::size(f);
            
            void * mem = a.allocate(f_sz);
            runtime::file::read(f, mem, sizeof(char), f_sz);
            
            return mem;
        }
    }
}