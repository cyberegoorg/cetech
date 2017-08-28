#include "celib/allocator.h"

#include "cetech/api/api_system.h"
#include "cetech/os/path.h"
#include "cetech/memory/memory.h"
#include "cetech/config/config.h"
#include "cetech/module/module.h"
#include "cetech/log/private/log_system_private.h"
#include "cetech/memory/private/memory_private.h"
#include "cetech/api/private/api_private.h"
#include "cetech/memory/private/allocator_core_private.h"

#include <cetech/application/application.h>

#include "celib/fpumath.h"


CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_module_a0);
CETECH_DECL_API(ct_app_a0);

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

int init_config(int argc,
                const char **argv) {
    auto kernel_platform = ct_config_a0.new_str("kernel.platform",
                                                "Kernel platform", _platform());
    ct_config_a0.new_str("kernel.native_platform", "Kernel native platform",
                         _platform());
    auto build = ct_config_a0.new_str("build", "Resource build dir",
                                      "build");

    if (!ct_config_a0.parse_args(argc, argv)) {
        return 0;
    }

    cel_alloc *a = ct_memory_a0.main_allocator();

    const char *build_dir_str = ct_config_a0.get_string(build);

    char *build_dir = ct_path_a0.join(a, 2, build_dir_str,
                                      ct_config_a0.get_string(kernel_platform));
    char *build_config = ct_path_a0.join(a, 2, build_dir, "global.config");

#ifdef CETECH_CAN_COMPILE
    ct_cvar source_dir = ct_config_a0.find("src");
    const char *source_dir_str = ct_config_a0.get_string(source_dir);
    char *source_config = ct_path_a0.join(a, 2, source_dir_str,
                                          "global.config");

    ct_cvar compile = ct_config_a0.find("compile");
    if (ct_config_a0.get_int(compile)) {
        ct_path_a0.make_path(build_dir);
        ct_path_a0.copy_file(a, source_config, build_config);
    }
    CEL_FREE(a, source_config);
#endif

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
    CETECH_LOAD_STATIC_MODULE(api, yamlng);
    CETECH_LOAD_STATIC_MODULE(api, config);
    CETECH_LOAD_STATIC_MODULE(api, object);
    CETECH_LOAD_STATIC_MODULE(api, watchdog);
    CETECH_LOAD_STATIC_MODULE(api, module);

    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_path_a0);
    CETECH_GET_API(api, ct_config_a0);
    CETECH_GET_API(api, ct_module_a0);

    init_config(argc, argv);

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
