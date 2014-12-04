#include "stdafx.h"
#include "DBDependencyEntry.h"


namespace l3d
{
	namespace packageinstaller
	{
		namespace db
		{
			DBDependencyEntry::DBDependencyEntry(int fileId, int packageId, bool installed)
				:fileId_(fileId), packageId_(packageId), installed_(installed)
			{
			}

			// FileID, PackageID, Installed,
			DBDependencyEntry::DBDependencyEntry(Kompex::SQLiteStatement& dbStmt) :
				fileId_(dbStmt.GetColumnInt(0)), packageId_(dbStmt.GetColumnInt(3)), installed_(dbStmt.GetColumnBool(2))
			{
			}

			int DBDependencyEntry::GetFileId() const
			{
				return fileId_;
			}
			int DBDependencyEntry::GetPackageId() const
			{
				return packageId_;
			}
			bool DBDependencyEntry::WasInstalled() const
			{
				return installed_;
			}

		}
	}
}