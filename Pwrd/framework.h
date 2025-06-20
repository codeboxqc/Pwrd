#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

 
#include <objidl.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib") // Link gdiplus.lib

using namespace Gdiplus;