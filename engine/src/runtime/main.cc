#include "common/device.h"

using namespace cetech;
int main(int argc, const char** argv) {
    device_globals::init();

    Device& d = device_globals::device();
    
    d.init(argc, argv);
    d.run();
    d.shutdown();

    device_globals::shutdown();

    return 0; // TODO: error check
}
