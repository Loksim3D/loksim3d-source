#include "stdafx.h"
#include "DBFileEntry.h"


namespace l3d
{
	namespace packageinstaller
	{
		namespace db
		{

			DBFileEntry::DBFileEntry(int id, int usageCount, std::wstring filepath, bool existedBefore)
				: id_(id), usageCount_(usageCount), filepath_(std::move(filepath)), existedBefore_(existedBefore)
			{
			}

			DBFileEntry::DBFileEntry(Kompex::SQLiteStatement& dbStmt) :
				id_(dbStmt.GetColumnInt(0)), usageCount_(dbStmt.GetColumnInt(3)), filepath_(dbStmt.GetColumnString16(1)), existedBefore_(dbStmt.GetColumnBool(2))
			{
			}

			int DBFileEntry::GetId() const
			{
				return id_;
			}
			int DBFileEntry::GetUsageCount() const
			{
				return usageCount_;
			}
			const std::wstring& DBFileEntry::GetFilepath() const
			{
				return filepath_;
			}
			bool DBFileEntry::ExistedBefore() const
			{
				return existedBefore_;
			}

		}
	}
}