#include "StdAfx.h"
#include "FileSystemUtils.h"

#include <Shlobj.h>

#include <lhWinAPI.h>

#include <boost/algorithm/string.hpp>

namespace l3d
{
namespace packageinstaller
{
namespace fs
{
		using namespace std;

		bool CreateDirectoriesTransacted(KTMTransaction& trans, const boost::filesystem::path& dir)
		{
			if (dir.empty() || FileExistsTransacted(trans, dir.c_str()))
			{
				if (!dir.empty() && !boost::filesystem::is_directory(dir))
				{
					// this is an error 
					// TODO log disp, whatever
				}
				return false;
			}

			// First create branch, by calling ourself recursively
			CreateDirectoriesTransacted(trans, dir.parent_path());
			// Now that parent's path exists, create the directory
			if (trans.CreateDirectoryExW(nullptr, dir.c_str(), nullptr) == 0)
			{
				std::wstring msg = lhWinAPI::GetLastErrorS();
				OutputDebugString(msg.c_str());
			}
			return true;
		}

		bool FileExistsTransacted(KTMTransaction& trans, const wchar_t *fileName)
		{
			WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
			if (trans.GetFileAttributesExW(fileName, GetFileExInfoStandard, &attr) != 0)
			{		
				return true;
			}
			return false;
		}

	void MoveDirectory(KTMTransaction& trans, const wchar_t* fromDir, const wchar_t* toDir, bool *cancel /* = nullptr */) 
	{
		if (cancel != nullptr && *cancel)
		{
			return;
		}

		trans.CreateDirectoryEx(nullptr, toDir, nullptr);
	
		WIN32_FIND_DATA FindFileData;
		HANDLE hFindFile;
	
		const static size_t BUF_SIZE = MAX_PATH * 2;
		std::unique_ptr<wchar_t> cszDirectoryFindPattern(new wchar_t[BUF_SIZE]);
		swprintf_s(cszDirectoryFindPattern.get(),BUF_SIZE, L"\\\\?\\%s\\*", fromDir);

		if ((hFindFile = trans.FindFirstFileEx(cszDirectoryFindPattern.get(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, nullptr, 0)) != INVALID_HANDLE_VALUE) 
		{
			do 
			{
				if (*FindFileData.cFileName == '.' && lstrlenW(FindFileData.cFileName) <= 2)
				{
					continue;
				}
			
				std::unique_ptr<wchar_t> cszFileOrDirectoryFrom(new wchar_t[BUF_SIZE]);
				std::unique_ptr<wchar_t> cszFileOrDirectoryTo(new wchar_t[BUF_SIZE]);
				swprintf_s(cszFileOrDirectoryFrom.get(), BUF_SIZE, L"%s\\%s", fromDir, FindFileData.cFileName);
				swprintf_s(cszFileOrDirectoryTo.get(), BUF_SIZE, L"%s\\%s", toDir, FindFileData.cFileName);
			
				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
				{
					MoveDirectory(trans, cszFileOrDirectoryFrom.get(), cszFileOrDirectoryTo.get());
					//trans.RemoveDirectory(cszFileOrDirectoryFrom);
				}
				else 
				{
					bool readOnly = false;
					WIN32_FILE_ATTRIBUTE_DATA fileAttr;
					if (GetFileAttributesEx(cszFileOrDirectoryTo.get(), GetFileExInfoStandard, &fileAttr))
					{
						if ((fileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
						{
							readOnly = true;
							trans.SetFileAttributes(cszFileOrDirectoryTo.get(), 
										fileAttr.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
						}
					}
					
					trans.MoveFileEx(cszFileOrDirectoryFrom.get(), cszFileOrDirectoryTo.get(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
					// auto err = lhWinAPI::GetLastErrorS();
					// OutputDebugString(err.c_str());
					if (readOnly)
					{
						trans.SetFileAttributes(cszFileOrDirectoryTo.get(), 
										fileAttr.dwFileAttributes | FILE_ATTRIBUTE_READONLY);
					}
				}
			} while(FindNextFile(hFindFile, &FindFileData) && (cancel == nullptr || *cancel == false));
			trans.RemoveDirectory(fromDir);
			FindClose(hFindFile);
		}
	}

	boost::filesystem::path LoksimRelativeToAbsolutePath(const std::wstring& relativePath, const boost::filesystem::path& l3dDir)
	{
		boost::filesystem::path p = relativePath;
		if (p.is_absolute())
		{
			return relativePath;
		}
		return (l3dDir / relativePath).make_preferred();
	}

	std::wstring AbsoluteToLoksimRelativePath(const boost::filesystem::path& absolutePath, const boost::filesystem::path& l3dDir)
	{
		wstring upperAbs = boost::algorithm::to_upper_copy(absolutePath.wstring());
		wstring upperL3d = boost::algorithm::to_upper_copy(l3dDir.wstring());

		boost::algorithm::replace_all(upperAbs, L"/", L"\\");
		boost::algorithm::replace_all(upperL3d, L"/", L"\\");
		if (upperL3d[upperL3d.size() - 1] != '\\')
		{
			upperL3d.append(L"\\");
		}

		if (upperAbs.find(upperL3d) == 0)
		{
			wstring ret = absolutePath.wstring().substr(upperL3d.length());
			if (ret[ret.size() - 1] == '\\')
			{
				return ret.substr(0, ret.size() - 1);
			}
			return ret;
		}
		return absolutePath.wstring();
	}

	// Liefert vollen Pfad zum Local-Settings Ordner des Loksims (LOCAL_APPDATA/Loksim3D)
	// Erstellt Verzeichnis falls es nicht existiert
	std::wstring GetLocalSettingsPfad()
	{
		static boost::filesystem::path localAppDataPath;

		if (localAppDataPath.empty())
		{
			std::unique_ptr<wchar_t[]> confpath(new wchar_t[MAX_PATH]);
			if (S_OK == SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, confpath.get()))
			{
				localAppDataPath = boost::filesystem::path(confpath.get()) /  L"Loksim3D";
				if (!boost::filesystem::exists(localAppDataPath))
				{
					boost::filesystem::create_directories(localAppDataPath);
				}
			}
		}
		return localAppDataPath.wstring();
	}

	// Liefert vollen Pfad für Datei in Local-Settings Ordner des Loksims (LOCAL_APPDATA/Loksim3D)
	// Erstellt Verzeichnis falls es nicht existiert
	std::wstring GetFilenameInLocalSettingsDir(const std::wstring& file)
	{
		static boost::filesystem::path localAppDataPath = GetLocalSettingsPfad();

		return (localAppDataPath / file).c_str();
	}
}
}
}