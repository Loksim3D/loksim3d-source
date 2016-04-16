#include "StdAfx.h"

#include "PreInstallWorker.h"
#include "PackageFileNameUtils.h"
#include "PackageSearch.h"

#include "Zip/unzip.h"
#include "InstallerException.h"
#include "L3dPackageInfo.h"

#include "L3dConsts.h"

#define SHA1_NO_WIPE_VARIABLES
#include "sha/SHA1.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


#include <lhTextFile.h>
#include <lhFileSystemUtils.h>
#include <lhException.h>
#include <unique_handle.h>
#include <regex>
#include <memory>
#include <iostream>

#include <Windows.h>

namespace l3d
{
namespace packageinstaller
{

using namespace std;
using namespace Concurrency;

PreInstallWorker::PreInstallWorker(const wstring& packageFileName)
{
	packageInfo.packageName = packageFileName;
}


PreInstallWorker::~PreInstallWorker(void)
{
	for (auto pkg = containedPackages.cbegin(); pkg != containedPackages.cend(); ++pkg)
	{
		// ursprüngliche boost variante eventuell nicht so gut, weil remove vermutlich exception wirft falls datei noch von virenscanner benutzt
		// windows deletefile löscht die datei in so einem fall einfach später
		if (!pkg->packageName.empty())
		{
			DeleteFile(pkg->packageName.c_str());
		}
		/*
		try
		{
			boost::filesystem::remove(pkg->packageName);
		} catch(...)
		{
			std::wcerr << L"PreInstallWorker::~PreInstallWorker - löschen von " << pkg->packageName << L" fehlgeschlagen" << endl;
			// Temp Datei kann nicht gelöscht werden => sollte nicht sein, aber auch nicht übermäßig tragisch
		}
		*/
	}
}

void PreInstallWorker::DoPreInstallWork()
{
	DoInnerWork(packageInfo);
	CSHA1 hasher;
	if (hasher.HashFile(packageInfo.packageName.c_str()))
	{
		hasher.Final();		
		hasher.ReportHashStl(packageInfo.hash);
	}
	else
	{
		throw lhstd::exception(std::wstring(L"Error calculation hash for ").append(packageInfo.packageName));
	}
}


void ThrowZipException(ZRESULT zipRes, const PackageInfo& pkgInfo, const char *exMsg, const std::wstring& curFile)
{
	wchar_t buf[256];
	FormatZipMessage(ZR_RECENT, buf, sizeof buf / sizeof buf[0]);
	InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorZipOccurred, pkgInfo, buf, curFile);
	throw InstallerException(errInfo, exMsg);
}

void PreInstallWorker::DoInnerWork(PackageInfo& curPkgInfo)
{
	unique_handle<HZIP, hzip_traits> hZip = unique_handle<HZIP, hzip_traits>(OpenZip(curPkgInfo.packageName.c_str(), nullptr));
	if (!hZip)
	{
		ThrowZipException(ZR_RECENT, curPkgInfo, "Error opening zip file", curPkgInfo.packageName);
	}

	ZIPENTRY ze;
	ZRESULT zRes;
	zRes = GetZipItem(hZip.get(), -1, &ze); 
	if (zRes != ZR_OK)
	{
		ThrowZipException(ZR_RECENT, curPkgInfo, "Error getting zip overview", curPkgInfo.packageName);
	}

	int numitems = ze.index;

	curPkgInfo.numFilesToInstall = 0;
	curPkgInfo.numElements = numitems;

	for (int i = 0; i < numitems && !is_current_task_group_canceling(); ++i)
	{ 
		zRes = GetZipItem(hZip.get(), i, &ze);
		if (zRes != ZR_OK)
		{
			ThrowZipException(ZR_RECENT, curPkgInfo, "Error getting zip item", curPkgInfo.packageName);
		}
		wstring fname = PackageFileNameToFileName(ze.name);
		if (boost::iequals(fname, L"Readme.txt"))
		{
			std::wstring tempName;
			if (lhWinAPI::GetTempFileNameInTempPath(L"l3d", 0, tempName) != 0)
			{
				std::wstring txt;
				UnzipItem(hZip.get(), i, tempName.c_str());
				{
					lhWinAPI::TextFile file(tempName, GENERIC_READ, 0, OPEN_EXISTING);
					file.ReadText(txt);
				}
				boost::filesystem::remove(tempName);
				if (packageInfo.readme.find(txt) == wstring::npos)
				{
					packageInfo.readme = packageInfo.readme.append(std::move(txt)).append(L"\r\n");//.append(L"\n\r\n\r*-*-*-*-*-*-*-*-*-*-*-*");	
				}
			}
		}
		else if (boost::iequals(fname, L"DeleteFiles.txt"))
		{
			std::wstring tempName;
			if (lhWinAPI::GetTempFileNameInTempPath(L"l3d", 0, tempName) != 0)
			{
				std::wstring txt;
				UnzipItem(hZip.get(), i, tempName.c_str());
				{
					lhWinAPI::TextFile file(tempName, GENERIC_READ, 0, OPEN_EXISTING);
					file.ReadText(txt);
				}
				boost::filesystem::remove(tempName);
				wregex re(L"[\\n\\r]+"); 
				regex_token_iterator<wstring::iterator> reg_it(txt.begin(), txt.end(), re, -1); 
				regex_token_iterator<wstring::iterator> end;
				while (reg_it != end)
				{
					if (std::find(begin(filesToDelete), std::end(filesToDelete), reg_it->str()) == std::end(filesToDelete))
					{
						filesToDelete.push_back(reg_it->str());
					}
					++reg_it;
				}
			}
		}
		else if (boost::iends_with(fname, L".l3dpack"))
		{
			std::wstring tempName;
			if (lhWinAPI::GetTempFileNameInTempPath(L"l3d", 0, tempName) != 0)
			{
				zRes = UnzipItem(hZip.get(), i, tempName.c_str());
				if (zRes != ZR_OK)
				{
					ThrowZipException(ZR_RECENT, curPkgInfo, "Error getting zip item for inner package", curPkgInfo.packageName);
				}
				PackageInfo pkgInfo;
				pkgInfo.packageName = tempName;
				pkgInfo.parentPackageName = curPkgInfo.packageName;
				DoInnerWork(pkgInfo);
				containedPackages.push_back(pkgInfo);
			}
		}
		else if ((boost::iequals(fname, "PackageInfo.xml")))
		{
			int versCode = -1;
			std::wstring tempName;
			if (lhWinAPI::GetTempFileNameInTempPath(L"l3d", 0, tempName) != 0)
			{
				UnzipItem(hZip.get(), i, tempName.c_str());
				{
					L3dPackageInfo inf;
					if (inf.LoadFromFile(tempName))
					{
						versCode = inf.GetVersionCode();
						wstring r = inf.GetFileInfo();
						if (packageInfo.readme.find(r) == wstring::npos)
						{
							packageInfo.readme = packageInfo.readme.append(r.append(L"\r\n"));
						}
						std::for_each(begin(inf.GetFilesToDelete()), end(inf.GetFilesToDelete()), [this](const std::wstring& s)
						{
							if (std::find(begin(filesToDelete), std::end(filesToDelete), s) == std::end(filesToDelete))
							{
								filesToDelete.push_back(s);
							}
						});
						
						if (!inf.GetPkgsToDeinstall().empty())
						{
							db::PackageSearch pkgSearch;
							std::for_each(begin(inf.GetPkgsToDeinstall()), end(inf.GetPkgsToDeinstall()), [this, &pkgSearch](const std::wstring& s)
							{
								auto res = pkgSearch.GetByGuid(s.substr(1));
								copy(begin(res), end(res), inserter(packagesToDeinstall, packagesToDeinstall.begin()));
							});

							copy(inf.GetPkgsToDeinstall().begin(), inf.GetPkgsToDeinstall().end(), inserter(packageGuidsToDeinstall, packageGuidsToDeinstall.begin()));
						}
					}
				}
				boost::filesystem::remove(tempName);

				if (versCode > LOKSIM3D_VERSION_CODE)
				{
					InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorPkgVersionNewer, curPkgInfo, L"", curPkgInfo.packageName);
					throw InstallerException(errInfo, "Package is newer than installed Loksim Version");
				}
			}
		}
		else if ((ze.attr & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			++curPkgInfo.numFilesToInstall;
		}
	}
}

}
}