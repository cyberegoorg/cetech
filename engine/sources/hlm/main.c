#include "hlm/application/application.h"

int main(int argc, char **argv) {
    const int init_result = application_init(argc, argv);

    if (init_result) {
        application_start();
    }

    application_shutdown();

    return !init_result;
}
