#pragma once

#include <string>

#include <KompexSQLiteStatement.h>

namespace l3d
{
	namespace packageinstaller
	{
		namespace db
		{

			class DBFileEntry
			{
			public:
				DBFileEntry(int id, int usageCount, std::wstring filepath, bool existedBefore);

				// Liest Daten aus Statement aus. Spalten müssen in folgender Reiehenfolge sein:
				// ID, Filename, ExistedBefore, UsageCount 
				explicit DBFileEntry(Kompex::SQLiteStatement& stmt);

				int GetId() const;
				int GetUsageCount() const;
				const std::wstring& GetFilepath() const;
				bool ExistedBefore() const;

			private:
				int id_;
				int usageCount_;
				std::wstring filepath_;
				bool existedBefore_;
			};

		}
	}
}