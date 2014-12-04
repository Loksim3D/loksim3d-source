#include "stdafx.h"
#include "DBPackageEntry.h"

#include <boost/filesystem.hpp>

namespace l3d
{
namespace packageinstaller 
{
namespace db
{
using namespace std;

DBPackageEntry::DBPackageEntry(Kompex::SQLiteStatement& dbStmt, int startCol) 
	: id(dbStmt.GetColumnInt(startCol + 0)), filepath(dbStmt.GetColumnString16(startCol + 1)),
	installedTimestamp(dbStmt.GetColumnInt64(startCol + 2)),
	readme(dbStmt.GetColumnString16(startCol + 3)), checksum(dbStmt.GetColumnString16(startCol + 4))
{
}

const std::wstring& DBPackageEntry::GetPackageName() const
{
	if (filename.length() == 0)
	{
		boost::filesystem::path p = filepath;
		filename = p.stem().wstring();
	}
	return filename;
};


bool operator==(const DBPackageEntry& en1, const DBPackageEntry en2)
{
	return en1.GetId() == en2.GetId() && en1.GetChecksum() == en2.GetChecksum();
}

bool operator<(const DBPackageEntry& en1, const DBPackageEntry en2)
{
	return en1.GetId() < en2.GetId() || en1.GetChecksum() < en2.GetChecksum();
}

}
}
}