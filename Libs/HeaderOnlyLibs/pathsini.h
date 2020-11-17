#ifndef PATHS_INI_GUARD_H
#define PATHS_INI_GUARD_H

#include <array>
#include <memory>

#include <Shlwapi.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "LoksimHandles.h"

namespace l3d::config {

namespace {
	std::wstring Convert(UINT CodePage, char* str)
	{
		int wchars_num = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
		std::vector<wchar_t> vecWstr = std::vector<wchar_t>(wchars_num + 1);
		MultiByteToWideChar(CP_UTF8, 0, str, -1, vecWstr.data(), wchars_num);
		return std::wstring(vecWstr.data());
	}
} // namespace

/**
 * Liest das Datenverzeichnis von Loksim3D aus der paths.ini
 * Falls nichts explizit gesetzt ist, oder paths.ini nicht existiert, wird l3dPath zurueckgeliefert
 * l3dPath: Absoluter Pfad zu Loksim-Verzeichnis
 */
boost::filesystem::path GetDataDirPathFromPathsIni(const boost::filesystem::path& l3dPath)
{
	boost::filesystem::path dataDir = l3dPath;
	auto hIniFile = l3d::util::make_HANDLE_unique_ptr(
	    CreateFile((l3dPath / L"\\paths.ini").c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL));
	std::array<char, 2048> buf;
	DWORD bytesRead = 0;
	if (ReadFile(hIniFile.get(), buf.data(), buf.size() - 1, &bytesRead, nullptr)) {
		buf[bytesRead] = buf[bytesRead + 1] = '\0';
		std::wstringstream strStream;
		std::wstring strData;
		for (int i = 0; i < 2; ++i) {
			if (i == 0) {
				if (bytesRead > 3 && buf[0] == '\xEF' && buf[1] == '\xBB' && buf[1] == '\xBF') {
					// strip BOM UTF-8
					strData = Convert(CP_UTF8, buf.data() + 3);
				}
				else {
					strData = Convert(CP_UTF8, buf.data());
				}
			}
			else {
				strData = reinterpret_cast<wchar_t*>(buf.data());
			}
			strStream = std::wstringstream(strData);
			try {
				boost::property_tree::wptree pt;
				boost::property_tree::ini_parser::read_ini(strStream, pt);
				std::wstring t = pt.get<std::wstring>(L"Directories.MainDir", l3dPath.c_str());
				if (t == L"<Registry>") {
					HKEY hKey;
					if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Loksim-Group\\Install", 0, KEY_QUERY_VALUE, &hKey) ==
					    ERROR_SUCCESS) {
						DWORD arSize = MAX_PATH;
						wchar_t tempPath[MAX_PATH];
						if (RegQueryValueEx(hKey, L"InstallDataDirPath", 0, 0, (LPBYTE) tempPath, &arSize) == ERROR_SUCCESS) {
							dataDir = tempPath;
						}
						RegCloseKey(hKey);
					}
				}
				else {
					dataDir = t;
				}
				if (dataDir.is_relative()) { dataDir = l3dPath / dataDir; }
				break;
			}
			catch (...) {
				dataDir = l3dPath;
			}
		}
	}
	wchar_t tempPath[MAX_PATH];
	if (PathCanonicalize(tempPath, dataDir.c_str()) && lstrlen(tempPath) > 1) {
		if (tempPath[lstrlen(tempPath) - 1] == '\\' && tempPath[lstrlen(tempPath) - 2] != ':') { tempPath[lstrlen(tempPath) - 1] = '\0'; }
		dataDir = tempPath;
	}
	return dataDir;
}
} // namespace l3d::config

#endif
