#include "..\Include\lhFileSystemUtils.h"

namespace lhWinAPI
{
	DWORD GetTempPath(std::wstring& dest)
	{
		TCHAR buf[MAX_PATH + 1];
		DWORD ret = ::GetTempPathW(MAX_PATH, buf);
		if (ret != 0 && ret < ARRAYSIZE(buf))
		{
			dest = buf;
			return ret;
		}
		return 0;
	}

	UINT GetTempFileName(const std::wstring& path, const std::wstring& prefix, UINT unique, std::wstring& dest)
	{
		TCHAR buf[MAX_PATH + 1];
		UINT ret = ::GetTempFileNameW(path.c_str(), prefix.c_str(), unique, buf);
		if (ret != 0)
		{
			dest = buf;
		}
		return ret;
	}

	UINT GetTempFileNameInTempPath(const std::wstring& prefix, UINT unique, std::wstring& dest)
	{
		std::wstring path;
		DWORD ret = GetTempPath(path);
		if (ret != 0)
		{
			return GetTempFileName(path, prefix, unique, dest);
		}
		return static_cast<UINT>(ret);
	}
}