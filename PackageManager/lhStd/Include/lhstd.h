#pragma once

#ifndef SAFE_DELETE
	#define SAFE_DELETE(x) if (x != 0) delete x; x = 0;
#endif

#include "lhstdG.h"

#if defined(UNICODE) || defined(_UNICODE)
	#include "lhstdW.h"
#else
	#include "lhstdA.h"
#endif