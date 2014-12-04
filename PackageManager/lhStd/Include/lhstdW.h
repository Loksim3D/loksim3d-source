#pragma once
#include <string>
#include <vector>

#define EXPORT __declspec (dllexport)

namespace lhstd
{
	//**************************************************************************
	// Splittet den String source immer beim Zeichen ch in den Vektor vDest auf
	EXPORT size_t split(std::vector<std::wstring>& vDest, const std::wstring& source, wchar_t ch);
}
