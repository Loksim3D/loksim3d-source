#pragma once

#include <KompexSQLiteException.h>
#include <KompexSQLiteStatement.h>
#include <KompexSQLiteDatabase.h>
#include "InstallHelperStructs.h"
#include "PackageInfo.h"
#include "AgentCommand.h"

#include <ppl.h>
#include <agents.h>
#include <memory>

#include <boost/filesystem.hpp>

namespace l3d
{
namespace packageinstaller
{
namespace db
{

/**
 * Kapselt Infos zu einer Datei bei der Package-Installation
 */
struct DbInstallFileInfo
{
	DbInstallFileInfo(const l3d::packageinstaller::insthelper::FileInstallInfo& _fileInfo, bool _installed) :
		fileInfo(_fileInfo), installed(_installed)
	{
	}

	/** 
	 * Infos über Datei
	 */
	l3d::packageinstaller::insthelper::FileInstallInfo fileInfo;
	/**
	 * true falls bei dieser Installation diese Datei installiert wurde
	 */
	bool installed;
};

/**
 * Agent welcher die Dateien eines Package in die DB einfügt
 */
class InstallPkgDBAgent : public Concurrency::agent
{
public:
	InstallPkgDBAgent(const boost::filesystem::path& l3dDir);
	~InstallPkgDBAgent(void);

	/**
	 * Liefert Puffer in welchen die zu installierenden Package-Informationen geschrieben werden können
	 */
	Concurrency::ITarget<RootPackageInfo*>* GetPkgBeginTarget() { return &pkgBeginBuffer; }
	/**
	 * Liefert Puffer in welchen die einzelnen Datei-Infos geschrieben werden können
	 */
	Concurrency::ITarget<std::shared_ptr<DbInstallFileInfo>>* GetFileInfoTarget() { return &fileInfoBuffer; }

	/**
	 * Aus diesem Puffer kann die Package-ID ausgelesen werden (nachdem eine Package-Info hinzugefügt wurde)
	 */
	Concurrency::ISource<int>* GetPkgDbIdSource() { return &pkgDbIdBuffer; }
	/**
	 * "Progress-Info": Anazhl verarbeiteter Dateien
	 */
	Concurrency::ISource<size_t>* GetCntProcessedFilesSource() { return &cntProcessedFilesBuffer; }

	/**
	 * Setzt ob Transaktion verwendet werden soll (ist abhängig davon ob auch NTFS-Transaktionen verwendet werden oder nicht)
	 */
	void SetUseTransaction(bool useTrans) { this->useTransaction = useTrans; }

	/**
	 * Setzt wie sich der Agent weiter verhalten soll (nach Abarbeitung stoppen, weitermachen, abbrechen)
	 */
	void SetAgentCommand(AgentCommand cmd);

	/**
	 * Wirft Exception falls bei der Abarbeitung der Packages/Dateien eine aufgetreten ist
	 */
	void ThrowExceptionIfAvailable();

	/**
	* Liefert die Connection zur Datenbank
	*/
	Kompex::SQLiteDatabase* GetConnection() const { return db.get(); }

protected:
	void run();

private:
	/**
	 * Nimmt die nötigen Queries an der DB vor, bevor die Installation der Dateien beginnen kann<br>
	 * Fügt das übergebene Package in die Package-Tabelle ein und setzt ID des Packages in rootPkgInfo
	 * @param rootPkgInfo Package das verarbeitet werden soll
	 */
	void PrepareDbBeforeInstall(l3d::packageinstaller::RootPackageInfo& rootPkgInfo);

	void DoInsertOrUpdateDependency(const l3d::packageinstaller::RootPackageInfo& rootPkg, int fileId, bool installed);

	void DoUpdateOrInsertDependency(const l3d::packageinstaller::RootPackageInfo& rootPkg, int fileId, bool installed);

	void WriteFileInstallInformationToDb(const l3d::packageinstaller::RootPackageInfo& rootPkg, const l3d::packageinstaller::insthelper::FileInstallInfo& fileInfo, bool installed);

	bool useTransaction;

	// outgoing buffers
	Concurrency::single_assignment<Kompex::SQLiteException> errorBuffer;
	Concurrency::unbounded_buffer<int> pkgDbIdBuffer;
	Concurrency::overwrite_buffer<size_t> cntProcessedFilesBuffer;

	// incomming buffers
	Concurrency::unbounded_buffer<RootPackageInfo*> pkgBeginBuffer;
	Concurrency::unbounded_buffer<std::shared_ptr<DbInstallFileInfo>> fileInfoBuffer;
	Concurrency::overwrite_buffer<AgentCommand> cmdBuffer;

	// DB Vars
	std::unique_ptr<Kompex::SQLiteDatabase> db;
	std::unique_ptr<Kompex::SQLiteStatement> stmtSelectFileId;
	std::unique_ptr<Kompex::SQLiteStatement> stmtInsertFile;
	std::unique_ptr<Kompex::SQLiteStatement> stmtInsertDependency;
	std::unique_ptr<Kompex::SQLiteStatement> stmtUpdateDependency;
	std::unique_ptr<Kompex::SQLiteStatement> stmtOther;

	/**
	 * Anzahl UNIQUE-Constraint-Violation Exceptions die beim Inserten von Dependencies bei diesem Package aufgetreten sind<br>
	 * Wird verwendet um zwischen Update- und Insert-Strategie umzustellen
	 */
	size_t cntDependencyInsertEx;

	size_t cntProcessedFiles;

	boost::filesystem::path l3dDir;
};

}
}
}