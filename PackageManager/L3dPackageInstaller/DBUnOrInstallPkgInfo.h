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
 * Kapselt wichtige Informationen über ein installiertes bzw deinstalliertes Package welche in der DB gespeichert sind<br>
 * Info: Es wird inzwischen in der DB nicht mehr gespeichert, welche Packages deinstalliert wurden => diese Klasse ist eigentlich nicht mehr in dieser Art notwendig aber sie erfüllt immer noch ihren Zweck
 * (IsInstalledPkg()) wird in der Praxis immer true liefern)
 */
class DBUnOrInstallPkgInfo
{
public:
	DBUnOrInstallPkgInfo(void) { }
	/**
	 * Liest die Daten mittels GetColumnXXX(int) aus dem Statement aus<br>
	 * Reihenfolge der Spalten muss stimmen (siehe Implementierung des Konstruktors)
	 */
	DBUnOrInstallPkgInfo(Kompex::SQLiteStatement& dbStmt);

	/**
	 * Liefert true falls dieses Objekt ein installiertes Package repräsentiert und false falls es ein deinstalliertes Package ist
	 */
	bool IsInstalledPkg() const { return isInstalledPkg; }
	int GetId() const { return id; }
	/**
	 * Liefert vollständigen Dateinamen des Package
	 */
	const std::wstring& GetPackagePath() const { return filepath; }
	const time_t& GetInstalledTimestamp() const { return installedTimestamp; }

	/**
	 * Liefert Dateinamen (ohne Pfad und ohne Endung) des Package
	 */
	const std::wstring& GetPackageName() const;

private:
	bool isInstalledPkg;
	int id;
	std::wstring filepath;
	time_t installedTimestamp;

	mutable std::wstring filename;
};

}
}
}