#ifndef OS_WINDOWS_MEM_INCLUDED
#define OS_WINDOWS_MEM_INCLUDED (1)

#include "dh/builtin.h"

#if bti_plat_windows
#undef auto
#undef var
#undef let
#undef warn_deprecated

#include <memoryapi.h>

#define auto            comp_syn__auto
#define var             comp_syn__var
#define let             comp_syn__let
#define warn_deprecated comp_attr__$warn_deprecated
#endif /* bti_plat_windows */

#endif /* OS_WINDOWS_MEM_INCLUDED */
