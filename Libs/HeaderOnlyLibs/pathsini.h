#ifndef PATHS_INI_GUARD_H
#define PATHS_INI_GUARD_H

#include <locale>
#include <codecvt>
#include <memory>

#include <Shlwapi.h>
		
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
		
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace l3d
{
	namespace config
	{
		/**
		 * Liest das Datenverzeichnis von Loksim3D aus der paths.ini
		 * Falls nichts explizit gesetzt ist, oder paths.ini nicht existiert, wird l3dPath zurueckgeliefert
		 * l3dPath: Absoluter Pfad zu Loksim-Verzeichnis
		 */
		boost::filesystem::path GetDataDirPathFromPathsIni(const boost::filesystem::path& l3dPath)
		{
			boost::filesystem::path p;
			std::unique_ptr<std::wifstream> fIni(new std::wifstream((l3dPath / L"\\paths.ini").c_str()));
			for (int i = 0; i < 2; ++i)
			{
				try 
				{
					boost::property_tree::wptree pt; 
					boost::property_tree::ini_parser::read_ini(*fIni, pt); 
					std::wstring t = pt.get<std::wstring>(L"Directories.MainDir", l3dPath.c_str());
					if (t == L"<Registry>") {
						HKEY hKey;
						if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Loksim-Group\\Install", 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
							DWORD arSize = MAX_PATH;
							wchar_t tempPath[MAX_PATH];
							if (RegQueryValueEx(hKey, L"InstallDataDirPath", 0, 0, (LPBYTE) tempPath, &arSize) == ERROR_SUCCESS) {
								p = tempPath;
							}
							RegCloseKey(hKey);
						}
					}
					else {
						p = t;
					}
					if (p.is_relative()) {
						p = l3dPath / p;
					}
					break;
				} catch (...)
				{	
					p = l3dPath;
					fIni.reset(new std::wifstream((l3dPath / L"\\paths.ini").c_str()));
					fIni->imbue(std::locale(fIni->getloc(),
						new std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>));
				}
			}	
			wchar_t tempPath[MAX_PATH];
			if (PathCanonicalize(tempPath, p.c_str()) && lstrlen(tempPath) > 1)
			{
				if (tempPath[lstrlen(tempPath)-1] == '\\' && tempPath[lstrlen(tempPath)-2] != ':')
				{
					tempPath[lstrlen(tempPath)-1] = '\0';
				}
				p = tempPath;
			}

			return p;
		}
	}
}

#endif