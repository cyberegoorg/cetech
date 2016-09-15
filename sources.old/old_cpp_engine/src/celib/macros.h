#pragma once

/*******************************************************************************
**** Macros
*******************************************************************************/

#define CE_UNUSED(x) do { (void)(x); } while (0)

#define CSTR_TO_ARG(str) str, sizeof(str)
#define STR_TO_ARG(str) str, strlen(str)
