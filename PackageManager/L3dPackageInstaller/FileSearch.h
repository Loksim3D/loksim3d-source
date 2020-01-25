#pragma once

#include <string>
#include <memory>
#include <utility>
#include <vector>

#include <KompexSQLiteDatabase.h>
#include <KompexSQLiteStatement.h>

#include "DBDependencyEntry.h"
#include "DBFileEntry.h"
#include "DBPackageEntry.h"


namespace l3d
{
	namespace packageinstaller
	{
		namespace db
		{


			/**
			* Ermöglicht Suche nach Dateien in DB
			*/
			class FileSearch
			{
			public:
				FileSearch();

				std::vector<DBFileEntry> SearchFilesByText(const std::wstring& txt);
				std::vector<std::pair<DBDependencyEntry, DBPackageEntry>> GetDependencies(int fileId);

			private:
				std::unique_ptr<Kompex::SQLiteDatabase> db_;
				std::unique_ptr<Kompex::SQLiteStatement> dbStmt_;
			};

		}
	}
}