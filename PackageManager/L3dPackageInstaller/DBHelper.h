#pragma once

#include <KompexSQLiteDatabase.h>
#include <KompexSQLiteStatement.h>

#include <string>
#include <memory>

#include <lhstd.h>

#include <boost/filesystem.hpp>

#include "DBUnOrInstallPkgInfo.h"

namespace l3d
{
namespace packageinstaller
{
namespace db
{

/**
 * Klasse welche Zugriffe auf DB kapselt / ermöglicht
 */
class DBHelper
{
public:
	~DBHelper(void);

	/**
	 * Singleton Intance
	 */
	static DBHelper& instance();

	/**
	 * Initialisiert die Datenbankverbindung und führt ev. notwendige DB-Update bzw DB-Create Operationen aus
	 * @throws SQLiteException bei DB-Fehler
	 */
	void InitDb(const boost::filesystem::path& dbPath, const boost::filesystem::path& l3dPath);

	/**
	 * Erstellt eine neue Connection zur DB die mittels InitDb initialisiert wurde
	 */
	std::unique_ptr<Kompex::SQLiteDatabase> GetOwnDbConnection(); 

	/**
	 * Liefert PackageName zu PackageID
	 */
	std::wstring GetPackageName(int packageId);

	std::wstring GetPackageBackupDir(int packageId);

	/**
	 * Falls die DB beim letzten Aufruf von InitDb neu angelegt wurde, wird hier true zurückgeliefert (entspricht erstem App-Start)
	 */
	bool IsDbNewCreated() const { return newDbCreated; }

	/**
	 * Liefert Einstellung aus DB
	 */
	std::wstring GetPreference(const std::wstring& prefName);
	/**
	 * Speichert Einstellung in DB
	 */
	void SetPreference(const std::wstring& prefName, const std::wstring& value);

	/**
	 * Liefert Einstellung aus DB wobei der als String gespeicherte Wert in einen anderen Typ konvertiert wird
	 */
	template <class ValueType>
	ValueType GetPreferenceValue(const std::wstring& prefName, ValueType defaultValue);

	/**
	 * Speichert Einstellung in DB, wobei der als beliebiger Typ übergebenen Wert von dieser Methode in einen String konvertiert wird
	 */
	template <class ValueType>
	void SetPreferenceValue(const std::wstring& prefName, const ValueType& value);

	/**
	 * Liefert einen Vektor mit allen in der DB gespeicherten installierten Packages
	 */
	std::vector<DBUnOrInstallPkgInfo> GetInstalledPackages();

	bool IsDbUpdateRequired();

	void UpdateDb(const boost::filesystem::path& l3dPath);

public:
	// Konstanten zur Identifikation von Einstellungen in DB
	static const std::wstring PREF_INSTALL_ALWAYS_ASK;
	static const std::wstring PREF_INSTALL_ONLY_OLDER;
	static const std::wstring PREF_DEINSTALL_ALWAYS_ASK;
	static const std::wstring PREF_ALLOW_UNDO;

protected:
	DBHelper(void);
	
	// Update Methoden für DB von einer Version zur nächsten
	void Update10To11();
	void Update11To12();
	void Update12To13(const boost::filesystem::path& l3dPath);
	void Update13To14();
	void Update14To15();

private:
	Kompex::SQLiteDatabase db;
	std::string utf8DbPath;
	std::unique_ptr<Kompex::SQLiteStatement> dbStmt;
	bool newDbCreated;
	boost::filesystem::path dbPath;

private:
	DBHelper(const DBHelper&);
	DBHelper& operator=(const DBHelper&);
};

template <class ValueType>
ValueType DBHelper::GetPreferenceValue(const std::wstring& prefName, ValueType defaultValue)
{
	return lhstd::Read<ValueType>(GetPreference(prefName), defaultValue);
}

template <class ValueType>
void DBHelper::SetPreferenceValue(const std::wstring& prefName, const ValueType& value)
{
	SetPreference(prefName, lhstd::String(value));
}

}
}
}