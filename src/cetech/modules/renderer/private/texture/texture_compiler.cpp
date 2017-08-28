#ifdef CETECH_CAN_COMPILE
//==============================================================================
// Include
//==============================================================================

#include "celib/allocator.h"
#include "celib/map.inl"

#include "cetech/kernel/hashlib.h"
#include "cetech/kernel/memory.h"
#include "cetech/kernel/api_system.h"
#include "cetech/kernel/log.h"
#include "cetech/modules/machine/machine.h"


#include "cetech/kernel/resource.h"
#include <cstdio>
#include <cetech/kernel/config.h>
#include "celib/buffer.inl"
#include "texture_blob.h"
#include "cetech/kernel/path.h"
#include "cetech/kernel/process.h"
#include "cetech/kernel/vio.h"

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
CETECH_DECL_API(ct_yamlng_a0);

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

    static int compiler(const char *filename,
                        struct ct_yamlng_document *doc,
                        ct_vio *build_vio,
                        ct_compilator_api *compilator_api) {

        auto a = ct_memory_a0.main_allocator();

        auto platform = ct_config_a0.find("kernel.platform");

        char output_path[1024] = {};
        char tmp_filename[1024] = {};

        const char *input_str = doc->get_string(doc->inst,
                                                ct_yamlng_a0.calc_key("input"),
                                                "");

        bool gen_mipmaps = doc->get_bool(doc->inst,
                                         ct_yamlng_a0.calc_key("gen_mipmaps"),
                                         true);

        bool is_normalmap = doc->get_bool(doc->inst,
                                          ct_yamlng_a0.calc_key("is_normalmap"),
                                          false);

        const char *source_dir = ct_resource_a0.compiler_get_source_dir();


        char *tmp_dir = ct_resource_a0.compiler_get_tmp_dir(a,
                                                            ct_config_a0.get_string(
                                                                    platform));
        char *input_path = ct_path_a0.join(a, 2, source_dir, input_str);

        _gen_tmp_name(output_path, tmp_dir, CETECH_ARRAY_LEN(tmp_filename),
                      input_str);

        int result = _texturec(input_path, output_path, gen_mipmaps,
                               is_normalmap);

        if (result != 0) {
            return 0;
        }

        ct_vio *tmp_file = ct_vio_a0.from_file(output_path,
                                               VIO_OPEN_READ);
        char *tmp_data =
                CEL_ALLOCATE(ct_memory_a0.main_allocator(), char,
                             tmp_file->size(tmp_file->inst) + 1);
        tmp_file->read(tmp_file->inst, tmp_data, sizeof(char),
                       tmp_file->size(tmp_file->inst));

        texture_blob::blob_t resource = {
                .size = (uint32_t) tmp_file->size(tmp_file->inst)
        };

        build_vio->write(build_vio->inst, &resource, sizeof(resource), 1);
        build_vio->write(build_vio->inst, tmp_data, sizeof(char),
                         resource.size);

        tmp_file->close(tmp_file->inst);

        compilator_api->add_dependency(filename, input_str);

        CEL_FREE(a, tmp_data);
        CEL_FREE(a, input_path);
        CEL_FREE(a, tmp_dir);

        return 1;
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
        CETECH_GET_API(api, ct_yamlng_a0);

        ct_resource_a0.compiler_register_yaml(
                ct_hash_a0.id64_from_str("texture"),
                compiler);

        return 1;
    }

}

#endif