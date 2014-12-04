#pragma once

#include <string>

namespace l3d
{
namespace packageinstaller
{

	/**
	 * Function that converts a filename used in package files to a real filename.
	 * It Converts Hex-Values in Filename to corresponding unicode characters
	 * @param packageFileName FileName in package
	 * @return Real filename
	 */
	std::wstring PackageFileNameToFileName(const std::wstring& packageFileName);

	/** Converts a windows filetime to c++ time_t
	 * @param ft Windows FileTime
	 * @return c++ time_t
	 */
	//time_t FileTimeToTime_t(const FILETIME& ft);

	/** Converts a unix filetime time_t to windows filetime
	 * @param t unix time
	 * @return windows filetime
	 */
	FILETIME UnixTimeToFileTime(time_t t);


}
}