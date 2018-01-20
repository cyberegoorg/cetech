#include "cetech/core/memory/allocator.h"

#include "cetech/core/api/api_system.h"
#include "cetech/core/os/path.h"
#include "cetech/engine/kernel/kernel.h"
#include "cetech/core/memory/memory.h"
#include "cetech/engine/config/config.h"
#include "cetech/core/module/module.h"
#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/log/private/log_system_private.h"
#include "cetech/core/memory/private/memory_private.h"
#include "cetech/core/api/private/api_private.h"
#include "cetech/core/memory/private/allocator_core_private.h"

#include <cetech/engine/application/application.h>
#include <cetech/core/coredb/coredb.h>


CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_coredb_a0);

#include <cetech/static_module.h>
#include <cetech/core/core.h>
#include <cetech/core/containers/buffer.h>

#define LOG_WHERE "kernel"

void register_api(struct ct_api_a0 *api);

const char *_platform() {
#if defined(CETECH_LINUX)
    return "linux";
#elif defined(CETECH_WINDOWS)
    return "windows";
#elif defined(CETECH_DARWIN)
    return "darwin";
#endif
    return NULL;
}

#define CONFIG_PLATFORM CT_ID64_0(CONFIG_PLATFORM_ID)
#define CONFIG_NATIVE_PLATFORM CT_ID64_0(CONFIG_NATIVE_PLATFORM_ID)

#define CONFIG_BUILD CT_ID64_0(CONFIG_BUILD_ID)
#define CONFIG_SRC CT_ID64_0(CONFIG_SRC_ID)
#define CONFIG_COMPILE CT_ID64_0(CONFIG_COMPILE_ID)

int init_config(int argc,
                const char **argv,
                struct ct_coredb_object_t *object) {
    struct ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(object);
    ct_coredb_a0.set_string(writer, CONFIG_PLATFORM, _platform());
    ct_coredb_a0.set_string(writer, CONFIG_NATIVE_PLATFORM, _platform());
    ct_coredb_a0.set_string(writer, CONFIG_BUILD, "build");
    ct_coredb_a0.write_commit(writer);

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    struct ct_alloc *a = ct_memory_a0.main_allocator();

    const char *build_dir_str = ct_coredb_a0.read_string(object, CONFIG_BUILD,
                                                         "");
    char *build_dir = NULL;
    ct_path_a0.join(&build_dir, a, 2,
                    build_dir_str,
                    ct_coredb_a0.read_string(object, CONFIG_NATIVE_PLATFORM, ""));

    char *build_config = NULL;
    ct_path_a0.join(&build_config, a, 2, build_dir, "global.config");

    const char *source_dir_str = ct_coredb_a0.read_string(object, CONFIG_SRC,
                                                          "");
    char *source_config = NULL;
    ct_path_a0.join(&source_config, a, 2, source_dir_str, "global.config");

    if (ct_coredb_a0.read_uint32(object, CONFIG_COMPILE, 0)) {
        ct_path_a0.make_path(build_dir);
        ct_path_a0.copy_file(a, source_config, build_config);
    }

    ct_config_a0.load_from_yaml_file(build_config, a);

    ct_buffer_free(source_config, a);
    ct_buffer_free(build_config, a);
    ct_buffer_free(build_dir, a);

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    return 1;
}

int cetech_kernel_init(int argc,
                       const char **argv) {
    celib_init();
    struct ct_api_a0 *api = api_v0();

    CETECH_LOAD_STATIC_MODULE(api, coredb);
    CETECH_LOAD_STATIC_MODULE(api, yamlng);
    CETECH_LOAD_STATIC_MODULE(api, config);
    CETECH_LOAD_STATIC_MODULE(api, ydb);
    CETECH_LOAD_STATIC_MODULE(api, module);

    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_config_a0);
    CETECH_GET_API(api, ct_module_a0);
    CETECH_GET_API(api, ct_coredb_a0);
    CETECH_GET_API(api, ct_hash_a0);


    init_config(argc, argv, ct_config_a0.config_object());


    CETECH_ADD_STATIC_MODULE(resourcesystem);
    CETECH_ADD_STATIC_MODULE(resourcecompiler);

    init_static_modules();
    CETECH_GET_API(api, ct_app_a0);

    ct_module_a0.load_dirs();
    ct_config_a0.log_all();

    return 1;
}


int cetech_kernel_shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");

    ct_module_a0.unload_all();

    struct ct_api_a0 *api = api_v0();

    CETECH_UNLOAD_STATIC_MODULE(api, config);
    CETECH_UNLOAD_STATIC_MODULE(api, filesystem);
    CETECH_UNLOAD_STATIC_MODULE(api, ydb);
    CETECH_UNLOAD_STATIC_MODULE(api, yamlng);

    celib_shutdown();

    return 1;
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        ct_app_a0.start();
    }

    return cetech_kernel_shutdown();
}
