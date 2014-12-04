#pragma once

#include "stdafx.h"
#include "TimeUtils.h"
#include <string>
#include <Windows.h>
#include <time.h>

/**
 * Formats Windows FileTime into readable date / time String
 * @param ft Time to format
 * @return formatted string
 */
std::wstring FormatFileTime(const FILETIME& ft)
{
	SYSTEMTIME st;
	wchar_t buf[256];
	std::wstring ret;

	FileTimeToSystemTime(&ft, &st);	
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, nullptr, buf, 256);

	ret = buf;
	GetTimeFormat(LOCALE_USER_DEFAULT, // predefined current user locale
		0, // option flag for things like no usage of seconds or // force 24h clock
		&st, // time - NULL to go with the current system locale time
		NULL, // time format string - NULL to go with default locale // 
		buf, // formatted string buffer
		256); // size of string buffer
	ret.append(L" ").append(buf);
	return ret;
}

/**
 * Formats Unix time_t into readable date / time String
 * @param t Time to format
 * @return formatted string
 */
std::wstring FormatUnixTime(const time_t& t)
{
	SYSTEMTIME st;
	SYSTEMTIME localSt;
	FILETIME ft;
	LONGLONG ll;
	wchar_t buf[256];
	std::wstring ret;
	
	// convert time_t to filetime
	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD)ll;
	ft.dwHighDateTime = ll >> 32;

	// convert filetime to systemtime
	FileTimeToSystemTime(&ft, &st);	
	SystemTimeToTzSpecificLocalTime(nullptr, &st, &localSt);

	// format date with default format
	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &localSt, nullptr, buf, 256);
	ret = buf;

	// format time with default format
	GetTimeFormat(LOCALE_USER_DEFAULT, // predefined current user locale
		0, // option flag for things like no usage of seconds or // force 24h clock
		&localSt, // time - NULL to go with the current system locale time
		NULL, // time format string - NULL to go with default locale // 
		buf, // formatted string buffer
		256); // size of string buffer
	ret.append(L" ").append(buf);
	return ret;
}

/**
 * Converts Windows FileTime to Unix time_t
 * @param ft Windows FileTime to convert
 * @return Unix time_t Timestamp
 */
time_t FileTimeToUnixTime(const FILETIME& ft)
{
	time_t ret;
	LONGLONG ll;
	ll = ft.dwHighDateTime;
	ll = ll << 32;
	ll = ll | ft.dwLowDateTime;
	ll = (ll - 116444736000000000) / 10000000;
	ret = ll;
	return ret;
}
