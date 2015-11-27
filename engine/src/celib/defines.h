#pragma once

#include "celib/compilers/generic/defines.h"

#if defined(CETECH_LINUX)
    #include "celib/compilers/gcc/defines.h"
#elif defined(CETECH_WINDOWS)
    #include "celib/compilers/msvc/defines.h"
#endif

