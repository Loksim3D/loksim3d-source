#include "stdafx.h"

#include <filesystem>
#include <regex>

#include <lhWinAPI.h>
#include <LoksimVersionHeader.h>

#include "VersionHelper.h"


namespace l3d::packageinstaller {

	int GetLoksimVersionCodeFromLoksimExe()
	{
		TCHAR filePath[MAX_PATH];
		GetModuleFileName(nullptr, filePath, MAX_PATH);

		std::filesystem::path pathToThisExe = filePath;
		auto pathToCheck = pathToThisExe.parent_path() / L"Loksim3D.exe";
		if (!std::filesystem::exists(pathToCheck)) {
			pathToCheck = pathToThisExe.parent_path() / L"LoksimEdit.exe";
		}

		if (std::filesystem::exists(pathToCheck)) {
			std::wstring version;
			std::wstring strFilePath(filePath);
			if (lhWinAPI::GetFileVersion(pathToCheck.wstring(), version, nullptr)) {
				std::wregex regex(L"(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
				std::wsmatch m;
				int versionCode = 0;
				if (std::regex_match(version, m, regex)) {
					versionCode += std::stoi(m[1]) * 1'000'000;
					versionCode += std::stoi(m[2]) *    10'000;
					versionCode += std::stoi(m[3]) *       100;
					return versionCode;
				}
			}
		}
		return L3D_VERSION_CODE;
	}
}