#pragma once

#if defined(_MSC_VER)
	#include <stdarg.h>
	#include <stdio.h>

	inline int vsnprintf_compat(char* buffer, size_t size, const char* format, va_list args)
	{
		int result = -1;
		if (size > 0)
			result = _vsnprintf_s(buffer, size, _TRUNCATE, format, args);
		if (result == -1)
			return _vscprintf(format, args);
	
		return result;	
	}

	inline int snprintf_compat(char* buffer, size_t size, const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		int result = vsnprintf_compat(buffer, size, format, args);
		va_end(args);
		return result;
	}

	#define snprintf snprintf_compat
	#define vsnprintf vsnprintf_compat
#endif
