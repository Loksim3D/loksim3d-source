#include "StdAfx.h"
#include "FileSearch.h"

#include <tuple>
#include <utility>


#include <KompexSQLiteException.h>

#include "DBFileEntry.h"
#include "DBHelper.h"
#include "DBPackageEntry.h"


namespace l3d
{
	namespace packageinstaller
	{
		namespace db
		{

			using namespace std;
			using namespace Kompex;

			FileSearch::FileSearch() : db_(DBHelper::instance().GetOwnDbConnection()), dbStmt_(new SQLiteStatement(db_.get()))
			{
			}


			//(ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
			std::vector<DBFileEntry> FileSearch::SearchFilesByText(const std::wstring& txt)
			{
				std::vector<DBFileEntry> ret;
				dbStmt_->Sql("SELECT ID, Filename, ExistedBefore, UsageCount FROM Files "
							"WHERE Filename LIKE(@txt) COLLATE NOCASE ORDER BY Filename LIMIT 10");
				dbStmt_->BindString16(1, txt.c_str());
				while (dbStmt_->FetchRow()) {
					ret.emplace_back(*dbStmt_);
				}
				dbStmt_->FreeQuery();
				return ret;
			}


			// Dependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID)
			// ID, Filename, InstallTimestamp, Readme, Checksum 
			std::vector<std::pair<DBDependencyEntry, DBPackageEntry>> FileSearch::GetDependencies(int fileId)
			{
				std::vector<std::pair<DBDependencyEntry, DBPackageEntry>> ret;
				dbStmt_->Sql("SELECT FileID, PackageID, Installed, ID, Filename, InstallTimestamp, Readme, Checksum FROM Dependencies "
							 "JOIN Packages ON PackageID=ID "
							 "WHERE FileID=@fileid ORDER BY InstallTimestamp DESC");
				dbStmt_->BindInt(1, fileId);
				while (dbStmt_->FetchRow()) {
					ret.emplace_back(
						std::piecewise_construct,
						std::forward_as_tuple(*dbStmt_),
						std::forward_as_tuple(*dbStmt_, 3));
				}
				dbStmt_->FreeQuery();
				return ret;
			}
		}
	}
}