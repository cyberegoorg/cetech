#include "engine/core/application.h"

int main(int argc,
         const char **argv) {

    if (application_init(argc, argv)) {
        application_start();
    }

    return application_shutdown();
}
