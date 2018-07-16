#ifndef CETECH_KERNEL_H
#define CETECH_KERNEL_H

//==============================================================================
// Include
//==============================================================================

#define CONFIG_PLATFORM \
     CT_ID64_0("kernel.platform", 0xa2e788cf39b7339bULL)

#define CONFIG_NATIVE_PLATFORM \
     CT_ID64_0("kernel.native_platform", 0xede1e0cf393fc17eULL)

#define CONFIG_BUILD \
     CT_ID64_0("build", 0x4429661936ece1eaULL)

#define CONFIG_SRC \
     CT_ID64_0("src", 0x1cdb3620898c588eULL)

#define CONFIG_CORE \
     CT_ID64_0("core", 0x6da99857e9315560ULL)

#define CONFIG_COMPILE \
     CT_ID64_0("compile", 0x3c797c340e1e5467ULL)

#define CONFIG_EXTERNAL \
     CT_ID64_0("external", 0x9fb8bb487a62dc4fULL)

#define CONFIG_MODULE_DIR \
     CT_ID64_0("module_dir", 0xa96daa49986032f4ULL)

#define CONFIG_BOOT_PKG \
     CT_ID64_0("core.boot_pkg", 0xd065c28a0c45037eULL)

#define CONFIG_CONTINUE \
     CT_ID64_0("continue", 0xb3326d55cc9374b8ULL)

#define CONFIG_WAIT \
     CT_ID64_0("wait", 0x21778fd44dd2ab6aULL)

#define CONFIG_GAME \
     CT_ID64_0("game", 0x8a426cdbe2604989ULL)

#define CONFIG_DAEMON \
     CT_ID64_0("daemon", 0xc3b953e09c1d1f60ULL)

#define CONFIG_WID \
     CT_ID64_0("screen.wid", 0xd950bcde34e3e4d7ULL)

#define CONFIG_SCREEN_X \
     CT_ID64_0("screen.x", 0xa59339bef4754edaULL)

#define CONFIG_SCREEN_Y \
     CT_ID64_0("screen.y", 0xc3492a4e07168b7aULL)

#define CONFIG_SCREEN_VSYNC \
     CT_ID64_0("screen.vsync", 0xfe469e6d1c14c940ULL)

#define CONFIG_SCREEN_FULLSCREEN \
     CT_ID64_0("screen.fullscreen", 0x613e9a6a17148a72ULL)

#define KERNEL_EVENT_DT \
    CT_ID64_0("dt", 0xbd04987fa96a9de5ULL)


#define SOURCE_ROOT \
    CT_ID64_0("source", 0x921f1370045bad6eULL)

#define BUILD_ROOT \
    CT_ID64_0("build", 0x4429661936ece1eaULL)

enum {
    KERNEL_EBUS = 0x6a0c4eb6
};

enum {
    KERNEL_INVALID_EVNT = 0,
    KERNEL_INIT_EVENT,
    KERNEL_UPDATE_EVENT,
    KERNEL_QUIT_EVENT,
    KERNEL_SHUTDOWN_EVENT,
};

enum {
    KERNEL_ORDER = 1024,
    GAME_ORDER = KERNEL_ORDER + 1024,
    RENDER = GAME_ORDER + 1024,
};

struct ct_app_update_ev {
    float dt;
};

#endif //CETECH_KERNEL_H
