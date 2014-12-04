#include "StdAfx.h"
#include "InstallerException.h"

namespace l3d
{
namespace packageinstaller
{

InstallerException::InstallerException(const InstallerErrorInformation& errInfo) : 
	exception(), errorInfo(errInfo)
{
}

InstallerException::InstallerException(const InstallerErrorInformation& errInfo, const char * const & _What) : 
	exception(_What), errorInfo(errInfo)
{
}

InstallerException::InstallerException(const DeinstallException& deinstallException) :
	exception(deinstallException), errorInfo(InstallerErrorInformation::InstallErrorDeinstall, PackageInfo(), deinstallException.GetMsg(), deinstallException.GetProcessedFilename())
{
}

InstallerException::~InstallerException(void)
{
}

}
}
