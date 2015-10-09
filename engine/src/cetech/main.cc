#include "celib/memory/memory.h"
#include "cetech/application/application.h"

using namespace cetech;
int main(int argc, const char** argv) {

    memory_globals::init();
    log::init();

    application_globals::init();
    Application& d = application_globals::app();

    d.init(argc, argv);

    log::info("Sdasd", "v3 a: %d s: %d", alignof(cetech::Vector3), sizeof(cetech::Vector3));
    log::info("Sdasd", "v4 a: %d s: %d", alignof(cetech::Vector4), sizeof(cetech::Vector4));

    d.run();
    d.shutdown();

    application_globals::shutdown();

    log::shutdown();
    memory_globals::shutdown();


    return 0; // TODO: error check
}
