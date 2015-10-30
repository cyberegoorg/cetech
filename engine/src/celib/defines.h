#pragma once

#include "celib/compilers/generic/defines.h"

#ifdef CETECH_LINUX
    #include "celib/compilers/gcc/defines.h"
#elif CETECH_WINDOWS
    #include "celib/compilers/msvc/defines.h"
#endif

