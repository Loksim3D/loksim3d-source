#include "StdAfx.h"
#include "PackageSearch.h"

#include <KompexSQLiteException.h>

#include "DBHelper.h"

namespace l3d
{
namespace packageinstaller
{
namespace db
{
using namespace std;
using namespace Kompex;

PackageSearch::PackageSearch(void) : db(DBHelper::instance().GetOwnDbConnection())
{
	dbStmt.reset(new SQLiteStatement(db.get()));
}


PackageSearch::~PackageSearch(void)
{
}

std::vector<DBPackageEntry> PackageSearch::GetAllPackages()
{
	std::vector<DBPackageEntry> ret;
	try
	{
		dbStmt->Sql("SELECT ID, Filename, InstallTimestamp, Readme, Checksum FROM Packages ORDER BY Filename");
		while (dbStmt->FetchRow())
		{
			ret.emplace_back(DBPackageEntry(*dbStmt));
		}
	} catch (const SQLiteException& ex)
	{
		OutputDebugStringA(ex.GetString().c_str());
	}
	dbStmt->FreeQuery();
	return ret;
}

std::vector<DBPackageEntry> PackageSearch::SearchByText(const std::wstring& txt)
{
	std::vector<DBPackageEntry> ret;
	dbStmt->Sql( "SELECT ID, Filename, InstallTimestamp, Readme, Checksum FROM Packages "
				"WHERE Filename LIKE @filename OR Readme LIKE @readme ORDER BY Filename");
	dbStmt->BindString16(1, txt.c_str());
	dbStmt->BindString16(2, txt.c_str());
	while (dbStmt->FetchRow())
	{
		ret.emplace_back(DBPackageEntry(*dbStmt));
	}
	dbStmt->FreeQuery();
	return ret;
}

// Suche anhand GUID
std::vector<DBPackageEntry> PackageSearch::GetByGuid(const std::wstring& guid)
{
	dbStmt->Sql("SELECT ID, Filename, InstallTimestamp, Readme, Checksum FROM Packages "
				"WHERE Checksum = @guid");
	dbStmt->BindString16(1, guid.c_str());
	std::vector<DBPackageEntry> ret;
	if (dbStmt->FetchRow())
	{
		ret.emplace_back(DBPackageEntry(*dbStmt));
	}
	dbStmt->FreeQuery();
	return ret;
}

}
}
}