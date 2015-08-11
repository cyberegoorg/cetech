#pragma once

#include <stdint.h>

#if defined(_MSC_VER)
	#define _ALLOW_KEYWORD_MACROS
#endif
	
#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif
