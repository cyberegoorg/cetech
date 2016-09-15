#include "engine/application/application.h"

int main(int argc,
         char **argv) {

    if (application_init(argc, argv)) {
        application_start();
    }

    return application_shutdown();
}
