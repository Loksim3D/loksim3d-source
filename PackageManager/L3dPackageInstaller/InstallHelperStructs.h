#pragma once

#include "TimeUtils.h"

#include <set>
#include <string>
#include <ppl.h>
#include <Windows.h>
#include <concurrent_vector.h>

namespace l3d
{
namespace packageinstaller
{
namespace insthelper
{

/**
 * Auswahl von Benutzer welche Dateien installiert und welche gelöscht werden sollen
 */
struct UserSelection
{
	std::set<std::wstring> filesToInstall;
	std::set<std::wstring> filesToDelete;
	std::set<int> packagesToDeinstall;
};

/**
 * Informationen über Datei welche installiert wird
 */
struct FileInstallInfo
{
	FileInstallInfo(const std::wstring& _fileName, const FILETIME& _modifiedTimePackage, const FILETIME& _modifiedTimeFileSystem) :
		fileName(_fileName),  modifiedTimePackage(FileTimeToUnixTime(_modifiedTimePackage)), modifiedTimeFileSystem(FileTimeToUnixTime(_modifiedTimeFileSystem))
	{
	}

	FileInstallInfo(const std::wstring& _fileName, const FILETIME& _modifiedTimePackage) :
		fileName(_fileName), modifiedTimePackage(FileTimeToUnixTime(_modifiedTimePackage))
	{
		modifiedTimeFileSystem = 0;
	}

	FileInstallInfo(const std::wstring& _fileName, const FILETIME& _modifiedTimeFileSystem, bool) :
		fileName(_fileName), modifiedTimeFileSystem(FileTimeToUnixTime(_modifiedTimeFileSystem))
	{
		modifiedTimePackage = 0;
	}

	FileInstallInfo(const std::wstring& _fileName) :
		fileName(_fileName), modifiedTimePackage(0), modifiedTimeFileSystem(0)
	{
	}

	bool FileExistsOnDisk() const
	{
		return modifiedTimeFileSystem != 0;
	}

	bool IsFileToInstall() const
	{
		return !IsFileToDeinstall() && modifiedTimePackage != 0;
	}

	bool IsFileToDelete() const
	{
		return !IsFileToInstall() && !IsFileToDeinstall();
	}

	bool IsFileToDeinstall() const
	{
		return modifiedTimeFileSystem == 0 && modifiedTimePackage == 0;
	}

	std::wstring fileName;
	time_t modifiedTimePackage;
	time_t modifiedTimeFileSystem;
};

bool operator<(const FileInstallInfo& i1, const FileInstallInfo& i2);


/**
 * Struct die Informationen über Installation kapselt
 */
struct InstallInformation
{
	std::wstring packageName;
	Concurrency::concurrent_vector<FileInstallInfo> filesToDelete;
	Concurrency::concurrent_vector<FileInstallInfo> writeProtectedFilesToDelete;
	Concurrency::concurrent_vector<FileInstallInfo> filesToInstall;
	Concurrency::concurrent_vector<FileInstallInfo> writeProtectedFilesToInstall;

	volatile size_t cntProcessedFiles;
};

}
}
}