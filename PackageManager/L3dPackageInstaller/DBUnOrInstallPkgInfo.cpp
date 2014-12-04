#include "StdAfx.h"
#include "DBUnOrInstallPkgInfo.h"

#include <boost/filesystem.hpp>

namespace l3d
{
namespace packageinstaller 
{
namespace db
{

DBUnOrInstallPkgInfo::DBUnOrInstallPkgInfo(Kompex::SQLiteStatement& dbStmt) : isInstalledPkg(dbStmt.GetColumnInt(0) == 0),
	id(dbStmt.GetColumnInt(1)), filepath(dbStmt.GetColumnString16(2)), installedTimestamp(dbStmt.GetColumnInt64(3))
{
}

const std::wstring& DBUnOrInstallPkgInfo::GetPackageName() const
{
	if (filename.length() == 0)
	{
		boost::filesystem::path p = filepath;
		filename = p.stem().wstring();
	}
	return filename;
};

}
}
}