#include "StdAfx.h"
#include "DBHelper.h"

#include <lhWinAPI.h>

#include <KompexSQLiteException.h>
#include <KompexSQLiteStatement.h>

#include <boost/filesystem.hpp>

#include "FileSystemUtils.h"

using namespace Kompex;

namespace l3d
{
namespace packageinstaller
{
	namespace db
	{
		using namespace std;


		const std::wstring DBHelper::PREF_INSTALL_ALWAYS_ASK = L"INSTALL_ALWAYS_ASK";
		const std::wstring DBHelper::PREF_INSTALL_ONLY_OLDER = L"INSTALL_ONLY_OLDER";
		const std::wstring DBHelper::PREF_DEINSTALL_ALWAYS_ASK = L"DEINSTALL_ALWAYS_ASK";
		const std::wstring DBHelper::PREF_ALLOW_UNDO = L"DEINSTALL_ALLOW_UNDO";

		DBHelper::DBHelper(void) : newDbCreated(false)
		{
		}


		DBHelper::~DBHelper(void)
		{
			try {
				db.Close();
			}
			catch (const SQLiteException& ex) {
				ex.Show();
			}
		}

		unique_ptr<SQLiteDatabase> DBHelper::GetOwnDbConnection()
		{
			return unique_ptr<SQLiteDatabase>(new SQLiteDatabase(utf8DbPath, SQLITE_OPEN_READWRITE, nullptr));
		}



		std::vector<DBUnOrInstallPkgInfo> DBHelper::GetInstalledPackages()
		{
			std::vector<DBUnOrInstallPkgInfo> ret;
			try {
				dbStmt->Sql("SELECT 0, ID, Filename, InstallTimestamp FROM Packages "
							"ORDER BY 4 DESC");
				while (dbStmt->FetchRow()) {
					ret.emplace_back(DBUnOrInstallPkgInfo(*dbStmt));
				}
			}
			catch (...) {
			}
			dbStmt->FreeQuery();
			return ret;
		}

		std::wstring DBHelper::GetPackageName(int packageId)
		{
			std::wstring res;
			try {
				dbStmt->Sql(L"SELECT Filename FROM Packages WHERE ID=@id");
				dbStmt->BindInt(1, packageId);
				if (dbStmt->FetchRow()) {
					res = dbStmt->GetColumnString16(0);
				}
			}
			catch (...) {
			}
			dbStmt->FreeQuery();
			return res;
		}

		std::wstring DBHelper::GetPackageBackupDir(int packageId)
		{
			std::wstring res;
			try {
				dbStmt->Sql("SELECT BackupDirectory FROM Packages WHERE ID=@pkgid");
				dbStmt->BindInt(1, packageId);
				if (dbStmt->FetchRow()) {
					res = dbStmt->GetColumnString16(0);
				}
			}
			catch (...) {
			}
			dbStmt->FreeQuery();
			return res;
		}

std::wstring DBHelper::GetPreference(const std::wstring& prefName)
{
	std::wstring res;
	try
	{
		dbStmt->Sql(L"SELECT Value FROM Preferences WHERE Name=@name");
		dbStmt->BindString16(1, prefName.c_str());
		if (dbStmt->FetchRow())
		{
			res = dbStmt->GetColumnString16(0);
		}
	} catch (...)
	{
	}
	dbStmt->FreeQuery();
	return res;
}

void DBHelper::SetPreference(const std::wstring& prefName, const std::wstring& value)
{
	std::wstring res;
	try
	{
		dbStmt->Sql("UPDATE Preferences Set Value = @value WHERE Name=@name;");
		dbStmt->BindString16(1, value.c_str());
		dbStmt->BindString16(2, prefName.c_str());

		dbStmt->ExecuteAndFree();
		
		if (db.GetDatabaseChanges() == 0)
		{
			dbStmt->Sql("INSERT INTO Preferences(Name, value) VALUES(@name, @value);");
			dbStmt->BindString16(1, prefName.c_str());
			dbStmt->BindString16(2, value.c_str());
			dbStmt->ExecuteAndFree();
		}
	} catch (const Kompex::SQLiteException& ex)
	{
		dbStmt->FreeQuery();
		throw ex;
	}
}

void DBHelper::Update14To15()
{
	SQLiteStatement stmt(&db);
	stmt.BeginTransaction();

	stmt.SqlStatement("CREATE INDEX Files_Filename_Index "
					  "ON Files(Filename COLLATE NOCASE)");


	stmt.SqlStatement("UPDATE VersionInfo SET Major=1, Minor=5 WHERE Component='Database'");

	stmt.CommitTransaction();
}

void DBHelper::Update13To14()
{
	{
		boost::system::error_code ec;
		boost::filesystem::copy_file(dbPath, dbPath.wstring() + L".bak", boost::filesystem::copy_option::overwrite_if_exists, ec);
	}

	SQLiteStatement stmt(&db);
	SQLiteStatement stmt2(&db);
	SQLiteStatement stmt3(&db);
	SQLiteStatement stmt4(&db);
	SQLiteStatement stmt5(&db);
	SQLiteStatement stmt6(&db);
	SQLiteStatement stmt7(&db);
	SQLiteStatement stmt8(&db);

	stmt2.BeginTransaction();

	stmt.Sql("SELECT LOWER(Filename), COUNT(Filename) FROM Files GROUP BY LOWER(Filename) HAVING COUNT(Filename) > 1;");
	stmt2.Sql("SELECT ID, Filename, ExistedBefore, UsageCount FROM Files WHERE LOWER(Filename) = LOWER(@filename);");
	stmt3.Sql("UPDATE Dependencies SET FileID=@fileId WHERE FileID=@searchId");
	stmt4.Sql("DELETE FROM Files WHERE ID=@searchId");
	stmt5.Sql("UPDATE Files SET ExistedBefore=@existedBefore, UsageCount=@usageCnt WHERE ID=@fileId");

	stmt6.Sql("SELECT PackageID FROM Dependencies WHERE FileID=@fileId");
	stmt7.Sql("UPDATE Dependencies SET FileID=@fileId WHERE FileID=@searchFileId AND PackageID=@searchPackageId");
	stmt8.Sql("DELETE FROM Dependencies WHERE FileID=@fileId AND PackageID=@packageId");
	while (stmt.FetchRow()) {
		wchar_t *fname = stmt.GetColumnString16(0);
		stmt2.BindString16(1, fname);

		if (stmt2.FetchRow()) {
			int id = stmt2.GetColumnInt(0);
			std::wstring fileName = stmt2.GetColumnString16(1);

			bool existedBefore = stmt2.GetColumnBool(2);
			int cnt = stmt2.GetColumnInt(3);

			while (stmt2.FetchRow()) {
				int innerId = stmt2.GetColumnInt(0);
				existedBefore = existedBefore && stmt2.GetColumnBool(2);
				cnt += stmt2.GetColumnInt(3);
				
				try {
					stmt3.BindInt(1, id);
					stmt3.BindInt(2, innerId);
					stmt3.Execute();
					stmt3.Reset();
				}
				catch (const Kompex::SQLiteException& ex) {
					OutputDebugStringA(ex.GetString().c_str());
					stmt3.FreeQuery();
					stmt3.Sql("UPDATE Dependencies SET FileID=@fileId WHERE FileID=@searchId");
					stmt6.BindInt(1, innerId);
					while (stmt6.FetchRow()) {
						try {
							stmt7.BindInt(1, id);
							stmt7.BindInt(2, innerId);
							stmt7.BindInt(3, stmt6.GetColumnInt(0));
							stmt7.Execute();
							stmt7.Reset();
						}
						catch (const Kompex::SQLiteException& ex2) {
							OutputDebugStringA(ex2.GetString().c_str());
							stmt7.FreeQuery();
							stmt7.Sql("UPDATE Dependencies SET FileID=@fileId WHERE FileID=@searchFileId AND PackageID=@searchPackageId");
							stmt8.BindInt(1, innerId);
							stmt8.BindInt(2, stmt6.GetColumnInt(0));
							stmt8.Execute();
							stmt8.Reset();
						}
					}
					stmt6.Reset();
				}

				stmt4.BindInt(1, innerId);
				stmt4.Execute();
				stmt4.Reset();
			}

			stmt5.BindBool(1, existedBefore);
			stmt5.BindInt(2, cnt);
			stmt5.BindInt(3, id);
		}
		stmt2.Reset();
	}
	stmt8.FreeQuery();
	stmt7.FreeQuery();
	stmt6.FreeQuery();
	stmt5.FreeQuery();
	stmt4.FreeQuery();
	stmt3.FreeQuery();
	stmt2.FreeQuery();
	stmt.FreeQuery();

	stmt2.SqlStatement("UPDATE VersionInfo SET Major=1, Minor=4 WHERE Component='Database'");

	stmt2.CommitTransaction();
}

void DBHelper::Update12To13(const boost::filesystem::path& l3dPath)
{
	SQLiteStatement stmt(&db);
	stmt.BeginTransaction();

	vector<pair<int, wstring>> updates;
	stmt.Sql("SELECT ID, BackupDirectory FROM Packages;");
	while (stmt.FetchRow())
	{
		wchar_t *backupDir = stmt.GetColumnString16(1);
		if (backupDir != nullptr && backupDir[0] != '\0')
		{
			updates.push_back(pair<int, wstring>(stmt.GetColumnInt(0), l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(backupDir, l3dPath)));
		}
	}
	stmt.FreeQuery();

	stmt.Sql("UPDATE Packages SET BackupDirectory=@dir WHERE ID=@id");
	for (auto iter = begin(updates); iter != end(updates); ++iter)
	{
		stmt.BindString16(1, iter->second.c_str());
		stmt.BindInt(2, iter->first);
		stmt.Execute();
		stmt.Reset();
	}
	stmt.FreeQuery();

	stmt.SqlStatement("UPDATE VersionInfo SET Major=1, Minor=3 WHERE Component='Database'");

	stmt.CommitTransaction();
}

void DBHelper::Update11To12()
{
	SQLiteStatement stmt(&db);
	stmt.BeginTransaction();

	stmt.SqlStatement("DROP TABLE IF EXISTS UninstalledFiles");
	stmt.SqlStatement("DROP TABLE IF EXISTS UninstalledPackages");
	stmt.SqlStatement("DROP TABLE IF EXISTS UninstalledDependencies");

	stmt.SqlStatement("UPDATE VersionInfo SET Major=1, Minor=2 WHERE Component='Database'");

	stmt.CommitTransaction();
}

void DBHelper::Update10To11()
{
	SQLiteStatement stmt(&db);
	stmt.BeginTransaction();
	
	stmt.SqlStatement("ALTER TABLE Packages ADD COLUMN BackupDirectory VARCHAR");

	/*
	stmt.SqlStatement("CREATE TABLE IF NOT EXISTS UninstalledFiles (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
	stmt.SqlStatement("CREATE TABLE IF NOT EXISTS UninstalledPackages (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL, InstallTimestamp Integer NOT NULL, Readme VARCHAR, Checksum INTEGER NOT NULL UNIQUE, BackupDirectory VARCHAR, UninstallBackupDirectory VARCHAR, UninstallTimestamp Integer)");
	stmt.SqlStatement("CREATE TABLE IF NOT EXISTS UninstalledDependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID))");

	stmt.SqlStatement("CREATE TRIGGER IF NOT EXISTS OnUninstalledDependencyInsert " 
						"AFTER INSERT ON UninstalledDependencies " 
						"FOR EACH ROW "
						"BEGIN "
						"UPDATE UninstalledFiles SET UsageCount = UsageCount + 1 WHERE ID = NEW.FileID; "
						"END ");

	stmt.SqlStatement("CREATE TRIGGER IF NOT EXISTS OnUninstalledDependencyDelete " 
						"AFTER DELETE ON UninstalledDependencies " 
						"FOR EACH ROW "
						"BEGIN "
						"UPDATE UninstalledFiles SET UsageCount = UsageCount - 1 WHERE ID = OLD.FileID AND UsageCount > 0; "
						"END ");*/

	stmt.SqlStatement("UPDATE VersionInfo SET Major=1, Minor=1 WHERE Component='Database'");

	stmt.CommitTransaction();
}

void DBHelper::InitDb(const boost::filesystem::path& dbPath, const boost::filesystem::path& l3dPath)
{
	std::string utf8Str;
	this->dbPath = dbPath;
	lhWinAPI::WideCharToMultiByte(CP_UTF8, 0, dbPath.c_str(), utf8Str, nullptr, nullptr);
	utf8DbPath = utf8Str;

	//boost::filesystem::remove(dbPath);
	try
	{
		db.Open(utf8Str, SQLITE_OPEN_READWRITE, nullptr);
	}
	catch (const SQLiteException&)
	{
		try 
		{
			if (!boost::filesystem::exists(dbPath.parent_path()))
			{
				boost::filesystem::create_directories(dbPath.parent_path());
			}

			db.Open(utf8Str, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
			SQLiteStatement stmt(&db);
			stmt.BeginTransaction();
			stmt.SqlStatement("CREATE TABLE Files (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
			stmt.SqlStatement("CREATE TABLE Packages (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL, InstallTimestamp Integer NOT NULL, Readme VARCHAR, Checksum INTEGER NOT NULL UNIQUE, BackupDirectory VARCHAR)");
			stmt.SqlStatement("CREATE TABLE Dependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID))");
			stmt.SqlStatement("CREATE TABLE VersionInfo (Component VARCHAR NOT NULL, Major INTEGER NOT NULL, Minor INTEGER NOT NULL, UNIQUE(Component))");
			stmt.SqlStatement("CREATE TABLE Preferences (Name VARCHAR NOT NULL PRIMARY KEY, Value VARCHAR NOT NULL)");

			/*
			stmt.SqlStatement("CREATE TABLE UninstalledFiles (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
			stmt.SqlStatement("CREATE TABLE UninstalledPackages (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL, InstallTimestamp Integer NOT NULL, Readme VARCHAR, Checksum INTEGER NOT NULL UNIQUE, BackupDirectory VARCHAR, UninstallBackupDirectory VARCHAR, UninstallTimestamp Integer)");
			stmt.SqlStatement("CREATE TABLE UninstalledDependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID))");
			*/

			stmt.SqlStatement("CREATE TRIGGER OnDependencyInsert " 
							  "AFTER INSERT ON Dependencies " 
							  "FOR EACH ROW "
							  "BEGIN "
							  "UPDATE Files SET UsageCount = UsageCount + 1 WHERE ID = NEW.FileID; "
							  "END ");

			stmt.SqlStatement("CREATE TRIGGER OnDependencyDelete " 
							  "AFTER DELETE ON Dependencies " 
							  "FOR EACH ROW "
							  "BEGIN "
							  "UPDATE Files SET UsageCount = UsageCount - 1 WHERE ID = OLD.FileID AND UsageCount > 0; "
							  "END ");

			stmt.SqlStatement("CREATE INDEX Files_Filename_Index "
							  "ON Files(Filename COLLATE NOCASE)");

			/*
			stmt.SqlStatement("CREATE TRIGGER OnUninstalledDependencyInsert " 
							  "AFTER INSERT ON UninstalledDependencies " 
							  "FOR EACH ROW "
							  "BEGIN "
							  "UPDATE UninstalledFiles SET UsageCount = UsageCount + 1 WHERE ID = NEW.FileID; "
							  "END ");

			stmt.SqlStatement("CREATE TRIGGER OnUninstalledDependencyDelete " 
							  "AFTER DELETE ON UninstalledDependencies " 
							  "FOR EACH ROW "
							  "BEGIN "
							  "UPDATE UninstalledFiles SET UsageCount = UsageCount - 1 WHERE ID = OLD.FileID AND UsageCount > 0; "
							  "END ");
			*/

			stmt.SqlStatement("INSERT INTO VersionInfo(Component, Major, Minor) VALUES('Database', 1, 5)");
			stmt.SqlStatement("INSERT INTO VersionInfo(Component, Major, Minor) VALUES('Programm', 2, 8)");
			stmt.CommitTransaction();

			newDbCreated = true;
		} catch (const SQLiteException& ex)
		{
			ex.Show();
			throw ex;
		}	
	}

	dbStmt.reset(new SQLiteStatement(&db));
}

bool DBHelper::IsDbUpdateRequired()
{
	SQLiteStatement stmt(&db);
	int major = stmt.GetSqlResultInt("SELECT Major FROM VersionInfo WHERE Component='Database'", -1);
	int minor = stmt.GetSqlResultInt("SELECT Minor FROM VersionInfo WHERE Component='Database'", -1);
	return major == 1 && minor < 5;
}

void DBHelper::UpdateDb(const boost::filesystem::path& l3dPath)
{
	SQLiteStatement stmt(&db);
	int major = stmt.GetSqlResultInt("SELECT Major FROM VersionInfo WHERE Component='Database'", -1);
	int minor = stmt.GetSqlResultInt("SELECT Minor FROM VersionInfo WHERE Component='Database'", -1);
	if (major == 1 && minor <= 0) {
		Update10To11();
	}
	if (major == 1 && minor <= 1) {
		Update11To12();
	}
	if (major == 1 && minor <= 2) {
		Update12To13(l3dPath);
	}
	if (major == 1 && minor <= 3) {
		Update13To14();
	}
	if (major == 1 && minor <= 4) {
		Update14To15();
	}
}


DBHelper& DBHelper::instance()
{
	static DBHelper inst;
	return inst;
}

}
}
}


/*
SQL-Statements



Version 1.0
			stmt.SqlStatement("CREATE TABLE Files (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
			stmt.SqlStatement("CREATE TABLE Packages (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL, InstallTimestamp Integer NOT NULL, Readme VARCHAR, Checksum INTEGER NOT NULL UNIQUE)");
			stmt.SqlStatement("CREATE TABLE Dependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID))");
			stmt.SqlStatement("CREATE TABLE VersionInfo (Component VARCHAR NOT NULL, Major INTEGER NOT NULL, Minor INTEGER NOT NULL, UNIQUE(Component))");
			stmt.SqlStatement("CREATE TABLE Preferences (Name VARCHAR NOT NULL PRIMARY KEY, Value VARCHAR NOT NULL)");

			stmt.SqlStatement("CREATE TRIGGER OnDependencyInsert " 
							  "AFTER INSERT ON Dependencies " 
							  "FOR EACH ROW "
							  "BEGIN "
							  "UPDATE Files SET UsageCount = UsageCount + 1 WHERE ID = NEW.FileID; "
							  "END ");

			stmt.SqlStatement("CREATE TRIGGER OnDependencyDelete " 
							  "AFTER DELETE ON Dependencies " 
							  "FOR EACH ROW "
							  "BEGIN "
							  "UPDATE Files SET UsageCount = UsageCount - 1 WHERE ID = OLD.FileID AND UsageCount > 0; "
							  "END ");

			stmt.SqlStatement("INSERT INTO VersionInfo(Component, Major, Minor) VALUES('Database', 1, 0)");
			stmt.SqlStatement("INSERT INTO VersionInfo(Component, Major, Minor) VALUES('Programm', 1, 0)");

*/