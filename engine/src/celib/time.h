#pragma once

#include <ctime>

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

        inline time_t get_sec(const PerfTimer& timer) {
            return timer.time.tv_sec;
        }

        inline long get_nsec(const PerfTimer& timer) {
            return timer.time.tv_nsec;
        }
    }
}