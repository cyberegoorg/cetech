#include "common/device.h"

using namespace cetech;
int main(int argc, const char** argv) {
    device_globals::init();
    
    Device& d = device_globals::device();
    
    device::init(d, argc, argv);
    device::run(d);
    device::shutdown(d);

    device_globals::shutdown();

    return 0; // TODO: error check
}
