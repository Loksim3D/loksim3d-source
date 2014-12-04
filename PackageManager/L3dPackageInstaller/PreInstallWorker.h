#pragma once

#include <vector>
#include <string>
#include <set>

#include <ppl.h>

#include "PackageInfo.h"
#include "DBPackageEntry.h"

namespace l3d
{
namespace packageinstaller
{


/**
 * Worker that examines L3D Package before Install to extract inner packages, delete list and readme
 */
class PreInstallWorker
{
public:
	PreInstallWorker(const std::wstring& packageFileName);
	~PreInstallWorker(void);

	/**
	 * Starts PreInstall Work
	 */
	void DoPreInstallWork();
	
	/**
	 * Liefert informationen zum Package
	 * @return PackageInfos
	 */
	inline RootPackageInfo& GetPackageInfo() { return packageInfo; }

	/**
	 * Liefert Event das gesetzt wird, falls Arbeit beendet ist
	 * @return event das nach Abschluss der Arbeit gesetzt ist
	 */
	inline Concurrency::event& GetWorkFinishedEvent() { return workFinished; }

	/**
	 * Liefert Informationen zu den beinhalteten Packages im Parent Package
	 * @return Vector von Packages im Package
	 */
	inline const std::vector<PackageInfo>& GetContainedPackages() const { return containedPackages; }

	/**
	 * Liefert Readme zu dem Package
	 * @return Readme von Package und eventuell beinhalteten Packages als String
	 */
	inline const std::wstring& GetReadme() const { return packageInfo.readme; }

	/**
	 * Liefert vector von zu löschenden Dateien (DeleteFiles.txt)
	 * @return Liste von zu löschenden Dateien
	 */
	inline const std::vector<std::wstring>& GetFilesToDelete() const { return filesToDelete; }
	
	/**
	 * Liefert set von zu löschenden Packages
	 * @return Set von zu löschenden Packages
	 */
	inline const std::set<db::DBPackageEntry>& GetPackagesToDeinstall() const { return packagesToDeinstall; }

private:
	void DoInnerWork(PackageInfo& curPkgInfo);

	RootPackageInfo packageInfo;

	Concurrency::event workFinished;
	/**
	* vector of all contained packages
	* pair.first is name of file in package
	* pair.second is filename of temp file where package was extracted to
	*/
	std::vector<PackageInfo> containedPackages;

	std::vector<std::wstring> filesToDelete;

	std::set<db::DBPackageEntry> packagesToDeinstall;
private:
	PreInstallWorker(const PreInstallWorker&);
	PreInstallWorker& operator=(const PreInstallWorker&);
};

}
}
