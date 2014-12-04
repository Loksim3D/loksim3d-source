#pragma once

#include <string>
#include <memory>
#include <vector>
#include <boost/noncopyable.hpp>

#include <KompexSQLiteDatabase.h>
#include <KompexSQLiteStatement.h>

#include "DBPackageEntry.h"

namespace l3d
{
namespace packageinstaller
{
namespace db
{

/**
 * Ermöglicht Suche nach Packages in DB
 */
class PackageSearch : boost::noncopyable
{
public:
	PackageSearch(void);
	~PackageSearch(void);

	/**
	 * Suche anhand eines Strings (sucht in Readme + Packagename)
	 */
	std::vector<DBPackageEntry> SearchByText(const std::wstring& txt);
	/**
	 * Liefert alle installierten Packages
	 */
	std::vector<DBPackageEntry> GetAllPackages();

	// Suche anhand GUID
	std::vector<DBPackageEntry> GetByGuid(const std::wstring& guid);

private:
	std::unique_ptr<Kompex::SQLiteDatabase> db;
	std::unique_ptr<Kompex::SQLiteStatement> dbStmt;
};


}
}
}