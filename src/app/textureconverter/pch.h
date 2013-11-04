#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define DISABLE_EXCEPTION_HANDLER
#ifdef _WIN32
#include <windows.h>
#endif  // _WIN32

#include <assert.h>
#include <systemlib/types.h>

#include <string>
#include <vector>
#include <stack>

#include "pystring.h"

#define Assert( cond )	assert( cond )

class InputImage;

typedef std::vector< InputImage* > InputImages;
typedef std::vector< std::string > StringVec;