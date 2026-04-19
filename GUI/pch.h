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
#include <tlhelp32.h>
#include <map>
#include <string>
#include <sstream>
#include <regex>
#include <thread>

#include "framework.h"

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

#include "GridCtrl.h"
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "GridCtrlD64.lib")
	#else
		#pragma comment(lib, "GridCtrl64.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "GridCtrlD.lib")
	#else
		#pragma comment(lib, "GridCtrl.lib")
	#endif
#endif

#pragma endregion

#endif //PCH_H
