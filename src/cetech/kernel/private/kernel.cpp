#include "celib/allocator.h"

#include "cetech/api/api_system.h"
#include "cetech/os/path.h"
#include "cetech/kernel/kernel.h"
#include "cetech/os/memory.h"
#include "cetech/config/config.h"
#include "cetech/module/module.h"
#include "cetech/hashlib/hashlib.h"
#include "cetech/log/private/log_system_private.h"
#include "cetech/os/private/memory_private.h"
#include "cetech/api/private/api_private.h"
#include "cetech/os/private/allocator_core_private.h"

#include <cetech/application/application.h>
#include <cetech/coredb/coredb.h>

#include "celib/fpumath.h"


CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_coredb_a0);

#include <cetech/static_module.h>

#define LOG_WHERE "kernel"

namespace os {
    void register_api(ct_api_a0 *api);
}

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

int init_config(int argc, const char **argv, ct_coredb_object_t *object) {
    ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(object);
    ct_coredb_a0.set_string(writer, CONFIG_PLATFORM, _platform());
    ct_coredb_a0.set_string(writer, CONFIG_NATIVE_PLATFORM, _platform());
    ct_coredb_a0.set_string(writer, CONFIG_BUILD, "build");
    ct_coredb_a0.write_commit(writer);

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    cel_alloc *a = ct_memory_a0.main_allocator();

    const char *build_dir_str = ct_coredb_a0.read_string(object, CONFIG_BUILD, "");

    char *build_dir = ct_path_a0.join(a, 2, build_dir_str,
                                      ct_coredb_a0.read_string(object, CONFIG_NATIVE_PLATFORM, ""));
    char *build_config = ct_path_a0.join(a, 2, build_dir, "global.config");

    const char *source_dir_str =  ct_coredb_a0.read_string(object, CONFIG_SRC, "");
    char *source_config = ct_path_a0.join(a, 2, source_dir_str,
                                          "global.config");

    if (ct_coredb_a0.read_uint32(object, CONFIG_COMPILE, 0)) {
        ct_path_a0.make_path(build_dir);
        ct_path_a0.copy_file(a, source_config, build_config);
    }
    CEL_FREE(a, source_config);

    ct_config_a0.load_from_yaml_file(build_config, a);

    CEL_FREE(a, build_config);
    CEL_FREE(a, build_dir);

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    return 1;
}

extern "C" int cetech_kernel_init(int argc,
                                  const char **argv) {
    auto *core_alloc = core_allocator::get();

    api::init(core_alloc);
    ct_api_a0 *api = api::v0();

    CETECH_LOAD_STATIC_MODULE(api, log);

    memory::init(4 * 1024 * 1024);

    core_allocator::register_api(api);

    CETECH_LOAD_STATIC_MODULE(api, hashlib);

    memory::register_api(api);

    CETECH_LOAD_STATIC_MODULE(api, error);
    CETECH_LOAD_STATIC_MODULE(api, vio);
    CETECH_LOAD_STATIC_MODULE(api, process);
    CETECH_LOAD_STATIC_MODULE(api, cpu);
    CETECH_LOAD_STATIC_MODULE(api, time);
    CETECH_LOAD_STATIC_MODULE(api, thread);
    CETECH_LOAD_STATIC_MODULE(api, path);
    CETECH_LOAD_STATIC_MODULE(api, coredb);
    CETECH_LOAD_STATIC_MODULE(api, yamlng);
    CETECH_LOAD_STATIC_MODULE(api, config);
    CETECH_LOAD_STATIC_MODULE(api, object);
    CETECH_LOAD_STATIC_MODULE(api, blob);
    CETECH_LOAD_STATIC_MODULE(api, task);
    CETECH_LOAD_STATIC_MODULE(api, watchdog);
    CETECH_LOAD_STATIC_MODULE(api, filesystem);
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


extern "C" int cetech_kernel_shutdown() {
    ct_log_a0.debug(LOG_WHERE, "Shutdown");

    ct_module_a0.unload_all();

    auto *api = api::v0();

    CETECH_UNLOAD_STATIC_MODULE(api, error);
    CETECH_UNLOAD_STATIC_MODULE(api, vio);
    CETECH_UNLOAD_STATIC_MODULE(api, process);
    CETECH_UNLOAD_STATIC_MODULE(api, cpu);
    CETECH_UNLOAD_STATIC_MODULE(api, time);
    CETECH_UNLOAD_STATIC_MODULE(api, thread);
    CETECH_UNLOAD_STATIC_MODULE(api, path);
    CETECH_UNLOAD_STATIC_MODULE(api, config);
    CETECH_UNLOAD_STATIC_MODULE(api, object);
    CETECH_UNLOAD_STATIC_MODULE(api, module);
    CETECH_UNLOAD_STATIC_MODULE(api, blob);
    CETECH_UNLOAD_STATIC_MODULE(api, task);
    CETECH_UNLOAD_STATIC_MODULE(api, watchdog);
    CETECH_UNLOAD_STATIC_MODULE(api, filesystem);
    CETECH_UNLOAD_STATIC_MODULE(api, ydb);
    CETECH_UNLOAD_STATIC_MODULE(api, yamlng);
    CETECH_UNLOAD_STATIC_MODULE(api, module);

    api::shutdown();
    memory::memsys_shutdown();
    logsystem::shutdown();

    return 1;
}

int main(int argc,
         const char **argv) {

    if (cetech_kernel_init(argc, argv)) {
        ct_app_a0.start();
    }

    return cetech_kernel_shutdown();
}
