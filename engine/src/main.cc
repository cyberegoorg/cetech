#include "common/memory/memory.h"
#include "application/application.h"

using namespace cetech;
int main(int argc, const char** argv) {

    memory_globals::init();
    log::init();

    application_globals::init();
    Application& d = application_globals::app();

    d.init(argc, argv);
    d.run();
    d.shutdown();

    application_globals::shutdown();

    log::shutdown();
    memory_globals::shutdown();


    return 0; // TODO: error check
}
