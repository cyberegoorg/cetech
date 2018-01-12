//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"
#include "celib/map.inl"

#include "cetech/hashlib/hashlib.h"
#include "cetech/os/memory.h"
#include "cetech/api/api_system.h"
#include "cetech/log/log.h"
#include "cetech/machine/machine.h"


#include "cetech/resource/resource.h"
#include <cstdio>
#include <cetech/config/config.h>
#include <cetech/yaml/ydb.h>
#include <celib/blob.h>
#include <cetech/coredb/coredb.h>
#include "celib/buffer.inl"
#include "texture_blob.h"
#include "cetech/os/path.h"
#include "cetech/os/process.h"
#include "cetech/os/vio.h"

using namespace celib;
using namespace buffer;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_process_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_coredb_a0);

namespace texture_compiler {

    static int _texturec(const char *input,
                         const char *output,
                         int gen_mipmaps,
                         int is_normalmap) {

        celib::Buffer buffer(ct_memory_a0.main_allocator());

        char *texturec = ct_resource_a0.compiler_external_join(
                ct_memory_a0.main_allocator(),
                "texturec");

        buffer << texturec;
        CEL_FREE(ct_memory_a0.main_allocator(), texturec);

        printf(buffer, " -f %s -o %s", input, output);

        if (gen_mipmaps) {
            buffer << " --mips";
        }

        if (is_normalmap) {
            buffer << " --normalmap";
        }

        buffer << " 2>&1";

        int status = ct_process_a0.exec(c_str(buffer));

        ct_log_a0.info("application", "STATUS %d", status);

        return status;
    }

    static int _gen_tmp_name(char *tmp_filename,
                             const char *tmp_dir,
                             size_t max_len,
                             const char *filename) {

        auto a = ct_memory_a0.main_allocator();

        char dir[1024] = {};
        ct_path_a0.dir(dir, filename);

        char *tmp_dirname = ct_path_a0.join(a, 2, tmp_dir, dir);
        ct_path_a0.make_path(tmp_dirname);

        int ret = snprintf(tmp_filename, max_len, "%s/%s.ktx", tmp_dirname,
                           ct_path_a0.filename(filename));

        CEL_FREE(a, tmp_dirname);

        return ret;
    }

    static void compiler(const char *filename,
                        struct ct_blob *output,
                        struct ct_compilator_api *compilator_api) {

        auto a = ct_memory_a0.main_allocator();

        auto platform = ct_coredb_a0.read_string(ct_config_a0.config_object(), CT_ID64_0("kernel.platform"), "");

        char output_path[1024] = {};
        char tmp_filename[1024] = {};

        uint64_t key[] = {
                ct_yng_a0.calc_key("input")
        };

        const char *input_str = ct_ydb_a0.get_string( filename, key, 1, "");

        key[0] = ct_yng_a0.calc_key("gen_mipmaps");
        bool gen_mipmaps = ct_ydb_a0.get_bool(filename, key, 1, true);

        key[0] = ct_yng_a0.calc_key("is_normalmap");
        bool is_normalmap = ct_ydb_a0.get_bool(filename, key, 1, false);

        const char *source_dir = ct_resource_a0.compiler_get_source_dir();

        char *tmp_dir = ct_resource_a0.compiler_get_tmp_dir(a, platform);
        char *input_path = ct_path_a0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _texturec(input_path, output_path, gen_mipmaps,
                               is_normalmap);

        if (result != 0) {
            return;
        }

        ct_vio *tmp_file = NULL;

        tmp_file = ct_vio_a0.from_file(output_path,
                            VIO_OPEN_READ);

        char *tmp_data =
                CEL_ALLOCATE(ct_memory_a0.main_allocator(), char,
                             tmp_file->size(tmp_file->inst) + 1);
        tmp_file->read(tmp_file->inst, tmp_data, sizeof(char),
                       tmp_file->size(tmp_file->inst));

        texture_blob::blob_t resource = {
                .size = (uint32_t) tmp_file->size(tmp_file->inst)
        };

        output->push(output->inst, &resource, sizeof(resource));
        output->push(output->inst, tmp_data, sizeof(char) * resource.size);

        tmp_file->close(tmp_file->inst);

        compilator_api->add_dependency(filename, input_str);

        CEL_FREE(a, tmp_data);
        CEL_FREE(a, input_path);
        CEL_FREE(a, tmp_dir);
    }

    int init(ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_resource_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_vio_a0);
        CETECH_GET_API(api, ct_process_a0);
        CETECH_GET_API(api, ct_log_a0);
        CETECH_GET_API(api, ct_hash_a0);
        CETECH_GET_API(api, ct_config_a0);
        CETECH_GET_API(api, ct_yng_a0);
        CETECH_GET_API(api, ct_ydb_a0);
        CETECH_GET_API(api, ct_coredb_a0);

        ct_resource_a0.compiler_register(
                CT_ID64_0("texture"),
                compiler, true);

        return 1;
    }

}
