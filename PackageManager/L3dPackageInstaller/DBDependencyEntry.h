#pragma once

#include <KompexSQLiteStatement.h>

namespace l3d
{
	namespace packageinstaller
	{
		namespace db
		{

			class DBDependencyEntry
			{
			public:
				DBDependencyEntry(int fileId, int packageId, bool installed);

				// Konstruiert aus Daten des Stmt
				// Spalten müssen in folgender Reihenfolge sein:
				// FileID, PackageID, Installed
				explicit DBDependencyEntry(Kompex::SQLiteStatement& stmt);

				int GetFileId() const;
				int GetPackageId() const;
				bool WasInstalled() const;

			private:
				int fileId_;
				int packageId_;
				bool installed_;
			};

		}
	}
}