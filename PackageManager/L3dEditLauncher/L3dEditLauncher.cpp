// L3dEditLauncher.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "L3dEditLauncher.h"
#include <memory>
#include <vector>
#include <string>
#include <Shlwapi.h>
#include <lhstd.h>
#include <algorithm>
#include <utility>
#include <iterator>
#include <set>
#include <Shellapi.h>
#include <lhWinAPI.h>

#include <pathsini.h>

#define BOOST_FILESYSTEM_NO_DEPRECATED

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;


boost::filesystem::path DetermineProcToStart(const boost::filesystem::path& filePath)
{
	wstring ext = filePath.extension().wstring();
	if (boost::algorithm::iequals(ext, ".l3dsky") ||
		boost::algorithm::iequals(ext, ".l3dwth"))
	{
		return L"LoksimWetterEdit.exe";
	}
	return L"LoksimEdit.exe";
}


boost::filesystem::path GetPathFromPathsIni(const boost::filesystem::path& l3dPath)
{	
	return l3d::config::GetDataDirPathFromPathsIni(l3dPath);
}


std::wstring GetMainInstallDirFromRegistry()
{
	DWORD dataSize;
	if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\Install", L"InstallPath", nullptr, nullptr, &dataSize) == ERROR_SUCCESS)
	{
		std::unique_ptr<wchar_t> data(new wchar_t[dataSize + 1]);
		if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\Install", L"InstallPath", nullptr, data.get(), &dataSize) == ERROR_SUCCESS)
		{
			return data.get();
		}
	}
	return L"";
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	if (__argc > 1)
	{
		wchar_t *fp = __wargv[1];
		//wchar_t *fp = L"D:\\Loksim3D_NewInst\\Objekte\\Strassen\\MartinF\\Feldweg.l3dobj";
		std::wstring tp = fp;
		boost::to_lower(tp);
		boost::filesystem::path boostPath = tp;
		boostPath = boostPath.make_preferred();
		wstring boostFileStr = boostPath.wstring();
		boost::filesystem::path procNameToStart = DetermineProcToStart(boostPath);

		// Zuerst überprüfen wir ob die Hauptinstallation für dieses Verzeichnis zuständig ist
		wstring mainl3ddir = GetMainInstallDirFromRegistry();
		if (!mainl3ddir.empty())
		{
			auto maindatadir = GetPathFromPathsIni(boost::filesystem::path(mainl3ddir)) / L"\\";
			wstring stringdir = maindatadir.c_str();
			boost::to_lower(stringdir);
			boost::filesystem::path pProc = mainl3ddir / procNameToStart;
			if (boost::starts_with(boostFileStr, stringdir) && PathFileExists(pProc.c_str()))
			{
				// "Hauptinstallation" (lt. Registry) ist für dieses Datenverzeichnis zuständig und .exe gefunden => hier starten
				ShellExecute(nullptr, nullptr, pProc.c_str(), lpCmdLine, nullptr, SW_SHOW);
				return 0;
			}
		}

		DWORD dataSize = 0;
		// Loksim Pfade aus Registry auslesen
		if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\Install", L"Paths", nullptr, nullptr, &dataSize) == ERROR_SUCCESS)
		{
			unique_ptr<wchar_t> data(new wchar_t[dataSize + 1]);
			if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\Install", L"Paths", nullptr, data.get(), &dataSize) == ERROR_SUCCESS)
			{
				wstring regStr = data.get();
				vector<wstring> splitStr;
				lhstd::split(splitStr, regStr, '|');

				for (size_t i = 0; i < splitStr.size(); i++)
				{
					boost::to_lower(splitStr[i]);
				}

				// pair.first = Verzeichnis der .exe Datei
				// pair.second = Datenverzeichnis für welches .exe Datei zuständig ist (lt. paths.ini)
				set<pair<wstring, wstring>> paths;
				auto backIns = inserter(paths, paths.begin());

				transform(begin(splitStr), end(splitStr), backIns, [](const wstring& p) -> pair<wstring, wstring>
				{
					boost::filesystem::path boostp(p);
					wstring procdir = boostp.make_preferred().parent_path().wstring();

					// Wir müssen uns die paths.ini holen um zu sehen für welchen Ordner diese Loksim-Installation "zuständig" ist
					boost::filesystem::path newpath = GetPathFromPathsIni(boostp.parent_path()); 
					wstring datadir = newpath.wstring();
					boost::to_lower(datadir);
					return std::make_pair(procdir, datadir);
				});

				vector<pair<wstring, wstring>> lengthSortedPaths(begin(paths), end(paths));
				sort(begin(lengthSortedPaths), end(lengthSortedPaths), [](const pair<wstring, wstring>& p1, const pair<wstring, wstring>& p2) 
				{
					return p1.second.length() > p2.second.length();
				});
				
				auto found = find_if(begin(lengthSortedPaths), end(lengthSortedPaths), [&boostFileStr, &procNameToStart](const pair<wstring, wstring>& p) -> bool
				{
					if (boost::starts_with(boostFileStr, p.second))
					{
						boost::filesystem::path pProc = p.first / procNameToStart;
						return PathFileExists(pProc.wstring().c_str()) ? true : false;
					}
					return false;
				});

				if (found != end(lengthSortedPaths))
				{
					// Im Pfad wo die zu öffnende Datei liegt, wurde LoksimEdit gefunden => diesen LoksimEdit öffnen
					ShellExecute(nullptr, nullptr, (found->first / procNameToStart).wstring().c_str(), lpCmdLine, nullptr, SW_SHOW);
					return 0;
				}
			}
		}
		// Datei liegt irgendwo oder Loksim-Pfade nicht auslesbar => den LoksimEdit im aktuellen Verzeichnis starten
		wstring s = lhWinAPI::FilePathExec(procNameToStart.wstring());
		ShellExecute(nullptr, nullptr, s.c_str(), lpCmdLine, nullptr, SW_SHOW);
	}
	return 0;
}



