#include "stdafx.h"
#include "InstallHelperStructs.h"
#include <string>


namespace l3d
{
namespace packageinstaller
{
namespace insthelper
{


bool operator<(const FileInstallInfo& i1, const FileInstallInfo& i2)
{
	return i1.fileName < i2.fileName;
}

}
}
}