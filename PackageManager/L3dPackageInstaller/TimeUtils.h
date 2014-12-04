#pragma once

#include <string>
#include <Windows.h>
#include <time.h>

/**
 * Formats Windows FileTime into readable date / time String
 * @param ft Time to format
 * @return formatted string
 */
std::wstring FormatFileTime(const FILETIME& ft);

/**
 * Formats Unix time_t into readable date / time String
 * @param t Time to format
 * @return formatted string
 */
std::wstring FormatUnixTime(const time_t& t);

/**
 *  Converts Windows FileTime to Unix time_t
 * @param ft Windows FileTime to convert
 * @return Unix time_t Timestamp
 */
time_t FileTimeToUnixTime(const FILETIME& ft);
