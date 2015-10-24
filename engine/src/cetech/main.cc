#include "celib/memory/memory.h"
#include "cetech/log_system/log_system.h"
#include "cetech/application/application.h"

#include "celib/command_line/command_line.h"

using namespace cetech;


void big_init() {
    memory_globals::init();
    log_globals::init();

    application_globals::init();
}

void big_shutdown() {
    application_globals::shutdown();

    
    log_globals::shutdown();
    memory_globals::shutdown();
}

void parse_command_line(int argc, const char** argv) {
    command_line_globals::set_args(argc, argv);
}

int main(int argc, const char** argv) {
    big_init();

    Application& d = application_globals::app();

    d.init(argc, argv);

    d.run();

    d.shutdown();

    big_shutdown();

    return 0; // TODO: error check
}
