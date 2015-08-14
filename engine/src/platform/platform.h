#pragma once

#ifdef CETECH1_LINUX
    #include "linux/defines.h"
#elif CETECH1_WINDOWS
    #include "windows/defines.h"
#else
    #include "generic/defines.h"
#endif
