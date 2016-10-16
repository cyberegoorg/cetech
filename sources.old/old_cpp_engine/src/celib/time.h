#pragma once

// TODO: REWRITE to multiplatform!!!
#ifdef __MACH__

#define CLOCK_PROCESS_CPUTIME_ID 0

#include <sys/time.h>
//clock_gettime is not implemented on OSX
static int clock_gettime(int /*clk_id*/,
                         struct timespec* t) {
    struct timeval now;
    int rv = gettimeofday(&now, NULL);
    if (rv) {
        return rv;
    }

    t->tv_sec = now.tv_sec;
    t->tv_nsec = now.tv_usec * 1000;
    return 0;
}
#else
#include <ctime>
#endif

namespace cetech {
    namespace time {
        struct PerfTimer {
            timespec time;
        };


        inline PerfTimer get_perftime() {
            timespec temp;

            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &temp);

            return {
                       temp
            };
        }

        inline int64_t get_sec(const PerfTimer& timer) {
            return timer.time.tv_sec;
            return timer.time.tv_sec;
        }

        inline int64_t get_nsec(const PerfTimer& timer) {
            return timer.time.tv_nsec;
        }
    }
}
