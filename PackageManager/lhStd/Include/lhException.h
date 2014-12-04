#pragma once

#if defined(UNICODE) || defined(_UNICODE)
	#include "lhExceptionW.h"
#else
	#include "lhExceptionA.h"
#endif