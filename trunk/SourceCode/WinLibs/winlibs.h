//#define LIBSAPI __declspec( dllexport )
#pragma once

#ifdef BUILD_WINLIBS
#define WINLIBSAPI __declspec( dllexport )
#else 
#define WINLIBSAPI __declspec( dllimport )
#endif


#include <libs.h>

