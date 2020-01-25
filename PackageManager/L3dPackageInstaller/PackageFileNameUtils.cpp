#include "stdafx.h"
#include "PackageFileNameUtils.h"

#include <string>
#include <sstream>

using namespace std;

namespace l3d
{
namespace packageinstaller
{

bool GetHexValue(const wstring& str, size_t startIndex, size_t length, wchar_t& value)
{
	if (str.length() - startIndex < length + 1)
	{
		return false;
	}
	if (str[startIndex] != L'#')
	{
		return false;
	}
	value = 0;
	for(size_t i = 1; i < length + 1; ++i)
	{
		value *= 0x10;
		WCHAR digit = str[startIndex + i];
		WCHAR digitValue;
		if (digit >= L'0' && digit <= L'9')
		{
			digitValue = digit - L'0';
		}
		else if (digit >= L'A' && digit <= L'F')
		{
			digitValue = digit - L'A' + 0x0a;
		} else if (digit >= L'a' && digit <= L'f')
		{
			digitValue = digit - L'a' + 0x0a;
		}
		else
		{
			return false;
		}
		value += digitValue;
	}
	return true;
}


wstring PackageFileNameToFileName(const wstring& packageFileName)
{
	wstringstream strBuf;

	//Jetzt die Escapes auflösen
	for(size_t i = 0; i < packageFileName.length(); ++i)
	{
		wchar_t hexValue;
		if (GetHexValue(packageFileName, i, 4, hexValue))
		{
			strBuf << hexValue;
			i += 4;
		}
		else
		{
			strBuf << packageFileName[i];
		}
	}
	return strBuf.str();
}

/*
#define SECS_TO_FT_MULT 10000000

time_t FileTimeToTime_t(const FILETIME& ft)
{
	LARGE_INTEGER li;    
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;
	time_t ret;
	ret = li.QuadPart;
	ret /= SECS_TO_FT_MULT;
	return ret;
}
*/

FILETIME UnixTimeToFileTime(time_t t)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;
	FILETIME ret;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ret.dwLowDateTime = (DWORD)ll;
	ret.dwHighDateTime = ll >> 32;
	return ret;
}

}
}