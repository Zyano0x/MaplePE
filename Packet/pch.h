// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <map>
#include <set>
#include <string>
#include <sstream>

#include<intrin.h>
#pragma intrinsic(_ReturnAddress)

#ifdef _WIN64
#define FASTCALL_EDX_PADDING
#else
#define FASTCALL_EDX_PADDING void* edx,
#endif


#pragma region lib

#include "Common.h"
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "CommonD64.lib")
	#else
		#pragma comment(lib, "Common64.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "CommonD.lib")
	#else
		#pragma comment(lib, "Common.lib")
	#endif
#endif

#ifndef IS_DEBUG_MODE
#define IS_DEBUG_MODE true
#endif

#include "MemorySDK.h"
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "MemorySDKD64.lib")
	#else
		#pragma comment(lib, "MemorySDK64.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "MemorySDKD.lib")
	#else
		#pragma comment(lib, "MemorySDK.lib")
	#endif
#endif

#pragma endregion

#endif //PCH_H
