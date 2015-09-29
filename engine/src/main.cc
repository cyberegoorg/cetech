#include "common/memory/memory.h"
#include "device.h"

using namespace cetech;
int main(int argc, const char** argv) {

    memory_globals::init();
    log::init();
    device_globals::init();

    Device& d = device_globals::device();

    d.init(argc, argv);
    d.run();
    d.shutdown();

    device_globals::shutdown();
    memory_globals::shutdown();
    log::shutdown();

    return 0; // TODO: error check
}
