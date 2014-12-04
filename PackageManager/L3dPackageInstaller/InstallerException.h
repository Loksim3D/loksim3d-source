#pragma once
#include <exception>
#include <string>

#include "PackageInfo.h"
#include "DeinstallManager.h"

namespace l3d
{
namespace packageinstaller
{

/**
 * Informationen zu Fehler bei Installation
 */
struct InstallerErrorInformation
{
	/**
	 * Fehlerquelle
	 */
	enum InstallerError
	{
		InstallErrorFileSystemOccurred, InstallErrorZipOccurred, InstallErrorTxFsOccurred, InstallErrorPkgVersionNewer, InstallErrorDeinstall
	};

	InstallerErrorInformation(const InstallerError& kind, const PackageInfo& package, std::wstring errMsg = L"", std::wstring curFile = L"") :
		kindOfError(kind), currentPackage(package), errorMsg(errMsg), currentFile(curFile)
	{
	}

	InstallerError kindOfError;
	PackageInfo currentPackage;
	std::wstring errorMsg;
	std::wstring currentFile;
};


/**
 * Exception bei Installation
 */
class InstallerException :
	public std::exception
{
public:
	InstallerException(const InstallerErrorInformation& errInfo);
	InstallerException(const DeinstallException& deinstallException);
	InstallerException(const InstallerErrorInformation& errInfo, const char * const & _What);


	virtual ~InstallerException(void);

	inline const InstallerErrorInformation& GetErrorInfo() const { return errorInfo; }

private:
	InstallerErrorInformation errorInfo;
};

}
}
