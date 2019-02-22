#include "StdAfx.h"
#include "InstallPkgDBAgent.h"
#include "DBHelper.h"

#include <memory>

#include <KompexSQLiteDatabase.h>
#include <KompexSQLiteStatement.h>
#include <KompexSQLiteException.h>

#include "InstallHelperStructs.h"

#include "FileSystemUtils.h"

namespace l3d
{
namespace packageinstaller
{
namespace db
{

using namespace std;
using namespace Kompex;
using namespace Concurrency;
using namespace l3d::packageinstaller;
using namespace l3d::packageinstaller::insthelper;

InstallPkgDBAgent::InstallPkgDBAgent(const boost::filesystem::path& _l3dDir) : cntProcessedFiles(0), l3dDir(_l3dDir)
{
	send(this->cntProcessedFilesBuffer, cntProcessedFiles);
}

InstallPkgDBAgent::~InstallPkgDBAgent(void)
{
}


void InstallPkgDBAgent::run(void)
{
	try
	{
		db = DBHelper::instance().GetOwnDbConnection();
		stmtSelectFileId.reset(new SQLiteStatement(db.get()));
		stmtInsertFile.reset(new SQLiteStatement(db.get()));
		stmtInsertDependency.reset(new SQLiteStatement(db.get()));
		stmtUpdateDependency.reset(new SQLiteStatement(db.get()));
		stmtOther.reset(new SQLiteStatement(db.get()));

		stmtSelectFileId->Sql("SELECT ID FROM Files WHERE Filename=@filename COLLATE NOCASE;");
		stmtInsertFile->Sql("INSERT INTO Files(Filename, ExistedBefore) VALUES(@filename, @existedbefore);");
		stmtInsertDependency->Sql("INSERT INTO Dependencies(FileID, PackageID, Installed) VALUES(@fileid, @packageid, @installed);");
		stmtUpdateDependency->Sql("UPDATE Dependencies Set Installed = @installed WHERE FileID=@fileid AND PackageID=@packageid;");

		bool transStarted = false;
		AgentCommand cmd = AgentContinueWork;
		RootPackageInfo *pCurPkg = nullptr;
		do 
		{
			try
			{
				cntDependencyInsertEx = 0;
				pCurPkg = receive(pkgBeginBuffer);
				if (!transStarted)
				{
					transStarted = true;
					try
					{
						//if (useTransaction)
						//{
							stmtOther->BeginTransaction();
							stmtOther->FreeQuery();
						//}
					} catch (const SQLiteException& ex)
					{
						send(errorBuffer, ex);
						send(pkgDbIdBuffer, -1);
						break;
					}
				}
				if (pCurPkg != nullptr)
				{
					PrepareDbBeforeInstall(*pCurPkg);
					shared_ptr<DbInstallFileInfo> fileInf;
					do
					{
						fileInf = receive(fileInfoBuffer);
						if (fileInf)
						{
							WriteFileInstallInformationToDb(*pCurPkg, fileInf->fileInfo, fileInf->installed);
							++cntProcessedFiles;
							if (cntProcessedFiles % 50 == 0)
							{
								send(cntProcessedFilesBuffer, cntProcessedFiles);
								if (cmdBuffer.has_value())
								{
									cmd = cmdBuffer.value();
									if (cmd == AgentCancelAndCommit || cmd == AgentCancelAndRollback)
									{
										pCurPkg = nullptr;
										break;
									}
								}
							}
						}
					} while (fileInf);
				}
			} catch (const SQLiteException& ex)
			{
				send(errorBuffer, ex);
				send(pkgDbIdBuffer, -1);
				pCurPkg = nullptr;
			}
			send(cntProcessedFilesBuffer, cntProcessedFiles);
			if (pCurPkg == nullptr)
			{
				// wartet bis msg da ist
				cmd = cmdBuffer.value();
			}
			else if (cmdBuffer.has_value())
			{
				cmd = cmdBuffer.value();
			}
			switch (cmd)
			{
			case AgentCancelAndCommit:
				pCurPkg = nullptr;
				if (useTransaction)
				{
					stmtOther->CommitTransaction();
					stmtOther->FreeQuery();
				}
				break;
			case AgentCancelAndRollback:
				pCurPkg = nullptr;
				if (useTransaction)
				{
					stmtOther->RollbackTransaction();
					stmtOther->FreeQuery();
				}
				break;
			case AgentStopAndCommit:
				if (pCurPkg == nullptr && useTransaction)
				{
					stmtOther->CommitTransaction();
					stmtOther->FreeQuery();
				}
				break;
			case AgentStopAndRollback:
				if (pCurPkg == nullptr && useTransaction)
				{
					stmtOther->RollbackTransaction();
					stmtOther->FreeQuery();
				}
				break;
			}
		} while (pCurPkg != nullptr);
		if (!useTransaction)
		{
			// Wenn keine Transaktionen eingesetzt werden, alles commiten egal ob Error oder nicht
			stmtOther->CommitTransaction();
			stmtOther->FreeQuery();
		}
	} catch (const SQLiteException& ex)
	{
		send(errorBuffer, ex);
	} catch (...)
	{
	}
	send(pkgDbIdBuffer, -1);
	done();
}

void InstallPkgDBAgent::SetAgentCommand(AgentCommand cmd)
{
	send(cmdBuffer, cmd);
}

void InstallPkgDBAgent::ThrowExceptionIfAvailable()
{
	if (errorBuffer.has_value())
	{
		SQLiteException ex = errorBuffer.value();
		throw ex;
	}
}

void InstallPkgDBAgent::WriteFileInstallInformationToDb(const RootPackageInfo& rootPkg, const FileInstallInfo& fileInfo, bool installed)
{
	int fileId = -1;
	stmtSelectFileId->BindString16(1, fileInfo.fileName.c_str());
	if (stmtSelectFileId->FetchRow())
	{
		fileId = stmtSelectFileId->GetColumnInt(0);
		stmtSelectFileId->Reset();
	}
	else
	{
		stmtSelectFileId->Reset();
		stmtInsertFile->BindString16(1, fileInfo.fileName.c_str());
		stmtInsertFile->BindBool(2, fileInfo.FileExistsOnDisk());
		stmtInsertFile->Execute();
		fileId = static_cast<int>(db->GetLastInsertRowId());
		stmtInsertFile->Reset();
	}

	if (cntDependencyInsertEx < 10)
	{
		DoInsertOrUpdateDependency(rootPkg, fileId, installed);
	}
	else
	{
		DoUpdateOrInsertDependency(rootPkg, fileId, installed);
	}
}

void InstallPkgDBAgent::DoUpdateOrInsertDependency(const RootPackageInfo& rootPkg, int fileId, bool installed)
{
	stmtUpdateDependency->BindBool(1, installed);
	stmtUpdateDependency->BindInt(2, fileId);
	stmtUpdateDependency->BindInt(3, rootPkg.dbId);
	stmtUpdateDependency->Execute();
	stmtUpdateDependency->Reset();

	if (db->GetDatabaseChanges() < 1)
	{
		DoInsertOrUpdateDependency(rootPkg, fileId, installed);
	}
}

void InstallPkgDBAgent::DoInsertOrUpdateDependency(const RootPackageInfo& rootPkg, int fileId, bool installed)
{
	stmtInsertDependency->BindInt(1, fileId);
	stmtInsertDependency->BindInt(2, rootPkg.dbId);
	stmtInsertDependency->BindBool(3, installed);
	stmtInsertDependency->Reset();
	try
	{
		stmtInsertDependency->Execute();
		stmtInsertDependency->Reset();
	} catch (const SQLiteException&)
	{
		++cntDependencyInsertEx;
		stmtInsertDependency->FreeQuery();
		stmtInsertDependency->Sql("INSERT INTO Dependencies(FileID, PackageID, Installed) VALUES(@fileid, @packageid, @installed);");

		// Dependency existiert bereits in DB => wir machen ein update bei dieser dependency
		stmtUpdateDependency->BindBool(1, installed);
		stmtUpdateDependency->BindInt(2, fileId);
		stmtUpdateDependency->BindInt(3, rootPkg.dbId);
		stmtUpdateDependency->Execute();
		stmtUpdateDependency->Reset();
	}
}

void InstallPkgDBAgent::PrepareDbBeforeInstall(RootPackageInfo& rootPkgInfo)
{				
	time_t t;
	time(&t);
	{
		wofstream outStr(fs::GetFilenameInLocalSettingsDir(fs::FILENAME_CRASHRPT_LOG), fstream::out | fstream::app);
		outStr << L"********************************" << endl;
		outStr << L"Filename " << rootPkgInfo.packageName << endl;
		outStr << L"InstallTimestamp " << t << endl;
		outStr << L"Readme " << rootPkgInfo.readme << endl;
		outStr << L"Checksum " << rootPkgInfo.hash << endl;
		outStr << L"BackupDirectory " << l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(rootPkgInfo.backupDirectory, l3dDir) << endl;
		outStr << L"********************************" << endl;
	}

	/*
		stmt.SqlStatement("ALTER TABLE Packages ADD COLUMN AddonId VARCHAR");
	stmt.SqlStatement("ALTER TABLE Packages ADD COLUMN AddonVersion INTEGER");
	stmt.SqlStatement("ALTER TABLE Packages ADD COLUMN Packageauthors VARCHAR");
	stmt.SqlStatement("ALTER TABLE Packages ADD COLUMN DisplayName VARCHAR");

	*/

	stmtOther->Sql("INSERT INTO Packages(Filename, InstallTimestamp, Readme, Checksum, BackupDirectory, AddonId, AddonVersion, Packageauthors, DisplayName) "
				   "VALUES(@filename, @time, @readme, @checksum, @backupdir, @addonid, @addonversion, @authors, @displayname);");
	stmtOther->BindString16(1, rootPkgInfo.packageName.c_str());
	stmtOther->BindInt64(2, t);
	stmtOther->BindString16(3, rootPkgInfo.readme.c_str());
	stmtOther->BindString16(4, rootPkgInfo.hash.c_str());
	stmtOther->BindString16(5, l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(rootPkgInfo.backupDirectory, l3dDir).c_str());
	stmtOther->BindString16(6, rootPkgInfo.addonId_.c_str());
	stmtOther->BindInt(7, rootPkgInfo.addonVersion_);
	stmtOther->BindString16(8, rootPkgInfo.packageAuthors_.c_str());
	stmtOther->BindString16(9, rootPkgInfo.displayName_.c_str());
	try {
		try {
			stmtOther->ExecuteAndFree();
			rootPkgInfo.dbId = static_cast<int>(db->GetLastInsertRowId());
		} 
		catch (const SQLiteException& e) {
			// unique checksum constraint violation
			stmtOther->FreeQuery();
			ofstream outStr(fs::GetFilenameInLocalSettingsDir(fs::FILENAME_CRASHRPT_LOG), fstream::out | fstream::app);
			outStr << L"Insert Package Exception" << endl;
			outStr << e.GetString() << endl;

			bool updateBackupDir = false;
			stmtOther->Sql("SELECT ID, BackupDirectory FROM Packages WHERE Checksum=@checksum;");
			stmtOther->BindString16(1, rootPkgInfo.hash.c_str());
			if (stmtOther->FetchRow()) {
				rootPkgInfo.dbId = stmtOther->GetColumnInt(0);
				wchar_t *backupDir = stmtOther->GetColumnString16(1);
				if (backupDir != nullptr) {
					rootPkgInfo.backupDirectory = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(backupDir, l3dDir).wstring();
				}
				else {
					updateBackupDir = true;
				}
			}
			else {
				throw std::logic_error("error fetching packageID from DB");
			}
			stmtOther->FreeQuery();

			if (updateBackupDir) {
				// Backup-Verzeichnis ist leer => Wir müssen BackupDir jz einfügen
				stmtOther->Sql("UPDATE Packages SET BackupDirectory=@dir WHERE ID=@id");
				stmtOther->BindString16(1, l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(rootPkgInfo.backupDirectory, l3dDir).c_str());
				stmtOther->BindInt(2, rootPkgInfo.dbId);
				stmtOther->ExecuteAndFree();
			}
		}

		send(pkgDbIdBuffer, rootPkgInfo.dbId);
	} 
	catch(...) {
		stmtOther->FreeQuery();
		send(pkgDbIdBuffer, -1);
		throw;
	}
}



}
}
}
