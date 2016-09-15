#ifndef CETECH_SYSTEMS_H
#define CETECH_SYSTEMS_H

//==============================================================================
// Defines
//==============================================================================

#define STATIC_SYSTEMS_SIZE sizeof(_SYSTEMS)/sizeof(_SYSTEMS[0])

//==============================================================================
// Magic
//==============================================================================

#define _SYSTEM_DECL

#include "_systems.h"

#undef _SYSTEM_DECL

#include "_systems.h"


#endif //CETECH_SYSTEMS_H
