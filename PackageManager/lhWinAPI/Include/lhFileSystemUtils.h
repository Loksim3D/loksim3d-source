#pragma once

#include <string>
#include <Windows.h>

namespace lhWinAPI
{
	DWORD GetTempPath(std::wstring& dest);
	
	UINT GetTempFileName(const std::wstring& path, const std::wstring& prefix, UINT unique, std::wstring& dest);

	UINT GetTempFileNameInTempPath(const std::wstring& prefix, UINT unique, std::wstring& dest);

}