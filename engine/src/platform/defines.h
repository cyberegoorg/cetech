#pragma once

#include "generic/defines.h"

#ifdef CETECH_LINUX
    #include "linux/defines.h"
#elif CETECH_WINDOWS
    #include "windows/defines.h"
#endif

