#pragma once

#include <string>

namespace l3d
{
namespace packageinstaller
{
	
/**
 * struct that encapsulates information about a package
 */
struct PackageInfo
{
	/**
	 * FileName of package
	 */
	std::wstring packageName;
	/**
	 * FileName of parent package (empty if no parent package exists)
	 */
	std::wstring parentPackageName;
	/**
	 * Number of elements in zip-file
	 */
	size_t numElements;

	/**
	 * Number of files that can / should be installed (does not inlude directories, readme files or inner-packages)
	 */
	size_t numFilesToInstall;
};

/**
 * struct that encapsulates information about a Root Package, i.e. a package that
 * was selected by user. In contrast to packages that are contained in other packages
 */
struct RootPackageInfo : public PackageInfo
{
	/**
	 * SHA1 Hash of package file
	 */
	std::wstring hash;
	/**
	 * ID of package in DB
	 */
	int dbId;
	/**
	 * aggregated readme of rootpackage and all contained packages
	 */
	std::wstring readme;

	/**
	* Backup Directory for install
	*/
	std::wstring backupDirectory;

	std::wstring displayName_;
	std::wstring addonId_;
	int addonVersion_;
	std::wstring packageAuthors_;
};

}
}
