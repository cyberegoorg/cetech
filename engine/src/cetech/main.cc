#include "celib/memory/memory.h"
#include "cetech/log_system/log_system.h"
#include "cetech/application/application.h"

using namespace cetech;

int main(int argc, const char** argv) {

    memory_globals::init();
    log_globals::init();

    application_globals::init();
    Application& d = application_globals::app();

    d.init(argc, argv);

    d.run();

    d.shutdown();

    application_globals::shutdown();

    log_globals::shutdown();
    memory_globals::shutdown();


    return 0; // TODO: error check
}
