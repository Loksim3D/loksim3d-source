#pragma once

#include <string>

#include <KompexSQLiteStatement.h>

namespace l3d
{
namespace packageinstaller 
{
namespace db
{

/**
 * Kapselt eine DB-Zeile welche ein installiertes Package in der DB repräsentiert
 */
class DBPackageEntry
{
public:
	DBPackageEntry(void) { }
	/**
	 * Liest die Daten mittels GetColumnXXX(int) aus dem Statement aus<br>
	 * Reihenfolge der Spalten muss stimmen (siehe Implementierung des Konstruktors)
	 */
	explicit DBPackageEntry(Kompex::SQLiteStatement& dbStmt, int startCol = 0);

	int GetId() const { return id; }
	/**
	 * Liefert vollständigen Dateinamen des Package
	 */
	const std::wstring& GetPackagePath() const { return filepath; }
	const time_t& GetInstalledTimestamp() const { return installedTimestamp; }
	const std::wstring& GetReadme() const { return readme; }
	const std::wstring& GetChecksum() const { return checksum; }

	/**
	 * Liefert Dateinamen (ohne Pfad und ohne Endung) des Package
	 */
	const std::wstring& GetPackageName() const;

private:
	int id;
	std::wstring filepath;
	time_t installedTimestamp;
	std::wstring readme;
	std::wstring checksum;

	mutable std::wstring filename;
};

bool operator==(const DBPackageEntry& en1, const DBPackageEntry en2);
bool operator<(const DBPackageEntry& en1, const DBPackageEntry en2);

}
}
}