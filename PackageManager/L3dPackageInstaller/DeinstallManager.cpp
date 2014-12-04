#include "StdAfx.h"
#include "DeinstallManager.h"

#include <algorithm>
#include <functional>
#include <sstream>
#include <set>

#include <lhWinAPI.h>
#include <KompexSQLiteStatement.h>
#include <KompexSQLiteException.h>

#include "TimeUtils.h"
#include "DBHelper.h"

#include "FileSystemUtils.h"

namespace l3d
{
namespace packageinstaller
{
using namespace std::placeholders; 

using namespace std;
using namespace Concurrency;
using namespace boost::filesystem;
using namespace Kompex;
using namespace l3d::packageinstaller::db;

/*------------------------------------------------------------------------*/

void ThrowableDeinstallAgent::ThrowExceptionIfAvailable()
{
	if (errBuffer.has_value())
	{
		throw errBuffer.value();
	}
}

/*------------------------------------------------------------------------*/

ProgressReportableAgent::ProgressReportableAgent(void) : cntTotalFiles(0)
{
}


size_t ProgressReportableAgent::GetProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles)
{
	if (cntTotalFiles != nullptr)
	{
		*cntTotalFiles = this->cntTotalFiles;
	}
	size_t processed = 0;
	if (cntProcessedFilesBuffer.has_value())
	{
		processed = cntProcessedFilesBuffer.value();
	}
	if (cntProcessedFiles != nullptr)
	{
		*cntProcessedFiles = processed;
	}
	if (this->cntTotalFiles > 0)
	{
		return min(100, (100 * processed / this->cntTotalFiles));
	}
	return 0;
}

/*------------------------------------------------------------------------*/

DeinstallAgent::DeinstallAgent(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete) :
	l3dPath(l3dPath), pkgIdsToDelete(pkgIdsToDelete), curAgent(), cntProcessedBefore(0), deletedFiles(make_shared<std::map<int, std::vector<std::wstring>>>()), undoInstallation(false),
	db(nullptr), transactFs(nullptr)
{
	KTMTransaction kt;
	usingTxFs = kt.UseTransactedFunctions();
	rootBackupPath = l3dPath / L"Backup\\Uninstall";
}


DeinstallAgent::DeinstallAgent(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete, Kompex::SQLiteDatabase* dbConnection, KTMTransaction* transactFs) :
	l3dPath(l3dPath), pkgIdsToDelete(pkgIdsToDelete), curAgent(), cntProcessedBefore(0), deletedFiles(make_shared<std::map<int, std::vector<std::wstring>>>()), undoInstallation(false),
	db(dbConnection), transactFs(transactFs)
{
	if (transactFs != nullptr)
	{
		usingTxFs = transactFs->UseTransactedFunctions();
	}
	else
	{
		KTMTransaction kt;
		usingTxFs = kt.UseTransactedFunctions();
	}
	rootBackupPath = l3dPath / L"Backup\\Uninstall";
}

DeinstallAgent::DeinstallAgent(const boost::filesystem::path& l3dPath,  std::shared_ptr<std::map<int, std::vector<std::wstring>>> packagesAndFilesToDelete) :
	l3dPath(l3dPath), curAgent(), packagesAndFilesToDelete(packagesAndFilesToDelete), cntProcessedBefore(0), undoInstallation(false),
	db(nullptr), transactFs(nullptr)
{
	transform(	begin(*packagesAndFilesToDelete), 
				end(*packagesAndFilesToDelete), 
				back_inserter(pkgIdsToDelete),
				[] (const std::map<int, std::vector<std::wstring>>::value_type& pair) { return pair.first; });

	KTMTransaction kt;
	usingTxFs = kt.UseTransactedFunctions();
	rootBackupPath = l3dPath / L"Backup\\Uninstall";
}

DeinstallAgent::~DeinstallAgent(void)
{
	std::shared_ptr<RemoveFilesAgent> a = curAgent;
	if (a)
	{
		agent::wait(a.get());
	}
}

void DeinstallAgent::SetAgentCommand(AgentCommand cmd)
{
	BaseDeinstallAgent::SetAgentCommand(cmd);
	if ((cmd == AgentCancelAndCommit || cmd == AgentCancelAndRollback))
	{
		std::shared_ptr<RemoveFilesAgent> a = curAgent;
		if (a)
		{
			a->SetAgentCommand(cmd);
		}
	}
}

size_t DeinstallAgent::GetProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles)
{
	std::shared_ptr<RemoveFilesAgent> a = curAgent;
	if (a)
	{
		send(cntProcessedFilesBuffer, cntProcessedBefore + a->GetCntProcessedFiles());
	}
	else
	{
		send(cntProcessedFilesBuffer, cntProcessedBefore);
	}
	return ProgressReportableAgent::GetProgress(cntProcessedFiles, cntTotalFiles);
}

/**
 * Liefert zurück ob es "eigene Transaktion" ist oder Transaktion von außen gesteuert wird
 */
bool DeinstallAgent::IsOwnTransaction() const
{
	return updb || db == nullptr;
}

void DeinstallAgent::run()
{
	AgentCommand cmd = AgentContinueWork;
	if (db == nullptr)
	{
		updb = DBHelper::instance().GetOwnDbConnection();
		db = updb.get();
	}
	SQLiteStatement dbStmt(db);
	bool cont = true;

	try
	{
		cntTotalFiles = 0;
		for_each(begin(pkgIdsToDelete), end(pkgIdsToDelete), [this, &dbStmt](int pkgId) {
			std::stringstream stream;
			stream << "SELECT COUNT(*) FROM Files, Dependencies WHERE Dependencies.PackageID=" << pkgId; 
			stream << " AND Dependencies.FileID=Files.ID AND Files.UsageCount == 1 AND Files.ExistedBefore == 0";
			cntTotalFiles += static_cast<size_t>(dbStmt.SqlAggregateFuncResult(stream.str()));
		});
	}
	catch(const SQLiteException& ex)
	{
		dbStmt.FreeQuery();
		cont = false;
		send(errBuffer, DeinstallException(-1, L"", ex.GetString()));
		done();
	}

	for (auto it = begin(pkgIdsToDelete); cont && it != end(pkgIdsToDelete); ++it)
	{
		int pkgId = *it;
		shared_ptr<RemoveFilesAgent> delAgent = make_shared<RemoveFilesAgent>(l3dPath, packagesAndFilesToDelete.get() == nullptr, transactFs);

		boost::filesystem::path backupDir;
		delAgent->SetUndoInstallation(undoInstallation);
		if (!undoInstallation)
		{
			dbStmt.Sql("SELECT Filename FROM Packages WHERE ID=@pkgid");
			dbStmt.BindInt(1, pkgId);
			boost::filesystem::path pkgPath;
			if (dbStmt.FetchRow())
			{
				pkgPath = dbStmt.GetColumnString16(0);
			}
			dbStmt.FreeQuery();
			backupDir = rootBackupPath / pkgPath.stem();
			int nr = 1;
			while (boost::filesystem::exists(backupDir))
			{
				backupDir = rootBackupPath / (pkgPath.stem().wstring() + lhstd::String(nr));
				nr++;
			}

			delAgent->SetBackupDir(backupDir);
		}
		else
		{
			dbStmt.Sql("SELECT BackupDirectory FROM Packages WHERE ID=@pkgid");
			dbStmt.BindInt(1, pkgId);
			if (dbStmt.FetchRow())
			{
				wchar_t *temp = dbStmt.GetColumnString16(0);
				if (temp != nullptr)
				{
					// Vorangestelltes \ beim backup-pfad entfernen
					while (temp[0] == '\\')
					{
						++temp;
					}
					backupDir = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(temp, l3dPath);
				}
				else
				{
					backupDir = L"";
				}
			}
			dbStmt.FreeQuery();
			delAgent->SetBackupDir(backupDir);
		}

		delAgent->start();
		curAgent = delAgent;
		try
		{
			if (IsOwnTransaction())
			{
				dbStmt.BeginTransaction();
			}
			dbStmt.FreeQuery();

//"CREATE TABLE UninstalledFiles (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
//"CREATE TABLE UninstalledPackages (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL, InstallTimestamp Integer NOT NULL, Readme VARCHAR, Checksum INTEGER NOT NULL UNIQUE, BackupDirectory VARCHAR, UninstallBackupDirectory VARCHAR, UninstallTimestamp Integer)");
//"CREATE TABLE UninstalledDependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID))");

//"CREATE TABLE Files (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL UNIQUE, ExistedBefore INTEGER NOT NULL DEFAULT 0, UsageCount INTEGER NOT NULL DEFAULT 0)");
//"CREATE TABLE Packages (ID INTEGER PRIMARY KEY NOT NULL, Filename VARCHAR NOT NULL, InstallTimestamp Integer NOT NULL, Readme VARCHAR, Checksum INTEGER NOT NULL UNIQUE, BackupDirectory VARCHAR NOT NULL)");
//"CREATE TABLE Dependencies (FileID INTEGER NOT NULL, PackageID INTEGER NOT NULL, Installed INTEGER NOT NULL DEFAULT 0, PRIMARY KEY(FileID, PackageID))");


			/*if (!undoInstallation)
			{
				dbStmt.Sql( "INSERT OR IGNORE INTO UninstalledPackages(ID, Filename, InstallTimestamp, Readme, Checksum, BackupDirectory) "
							"SELECT ID, Filename, InstallTimestamp, Readme, Checksum, BackupDirectory FROM Packages WHERE ID=@pkgid"); 
				dbStmt.BindInt(1, pkgId);
				dbStmt.ExecuteAndFree();

				time_t t;
				time(&t);
				dbStmt.Sql("UPDATE UninstalledPackages SET UninstallBackupDirectory=@backupDir, UninstallTimestamp=@time WHERE ID=@pkgid");
				dbStmt.BindString16(1, backupDir.c_str());
				dbStmt.BindInt64(2, t);
				dbStmt.BindInt(3, pkgId);
				dbStmt.ExecuteAndFree();

				dbStmt.Sql( "INSERT OR IGNORE INTO UninstalledFiles(ID, Filename, ExistedBefore) "
							"SELECT Files.ID, Files.Filename, Files.ExistedBefore "
							"FROM Files, Dependencies WHERE Dependencies.PackageID=@pkgid AND Dependencies.FileID=Files.ID" );
				dbStmt.BindInt(1, pkgId);
				dbStmt.ExecuteAndFree();


				dbStmt.Sql(	"INSERT OR IGNORE INTO UninstalledDependencies(FileID, PackageID, Installed) "
							"SELECT FileID, PackageID, Installed FROM Dependencies WHERE PackageID=@pkgid");
				dbStmt.BindInt(1, pkgId);
				dbStmt.ExecuteAndFree();
			}*/

			dbStmt.Sql("DELETE FROM Dependencies WHERE PackageID=@pkgid");
			dbStmt.BindInt(1, pkgId);
			dbStmt.ExecuteAndFree();

			if (packagesAndFilesToDelete)
			{
				auto v = (*packagesAndFilesToDelete)[pkgId];
				for_each(begin(v), end(v), [&delAgent](const wstring& f) { send(delAgent->GetFilesToDeleteTarget(), f); });
			}
			else
			{
				dbStmt.Sql("SELECT Filename FROM Files WHERE UsageCount == 0 AND ExistedBefore == 0 ORDER BY Filename");
				while (dbStmt.FetchRow())
				{
					wstring filename = dbStmt.GetColumnString16(0);
					send(delAgent->GetFilesToDeleteTarget(), filename);
				}
				dbStmt.FreeQuery();
			}

			dbStmt.SqlStatement("DELETE FROM Files WHERE UsageCount == 0");
		

			dbStmt.Sql("DELETE FROM Packages WHERE ID=@pkgid");
			dbStmt.BindInt(1, pkgId);
			dbStmt.ExecuteAndFree();

			delAgent->SetAgentCommand(AgentStopAndCommit);
			send<std::wstring>(delAgent->GetFilesToDeleteTarget(), L"");
			agent::wait(delAgent.get());

			if (!delAgent->HasError())
			{
				if (IsOwnTransaction())
				{
					dbStmt.CommitTransaction();
					dbStmt.FreeQuery();
				}
			}
			else
			{
				if (IsOwnTransaction())
				{
					dbStmt.RollbackTransaction();
					dbStmt.FreeQuery();
				}

				pair<wstring, wstring> err = delAgent->GetError();
				send(errBuffer, DeinstallException(pkgId, err.first, err.second));
			}

			if (cmdBuffer.has_value())
			{
				cmd = cmdBuffer.value();
			}
			delAgent->SetAgentCommand(cmd);
			if (cmd == AgentCancelAndCommit || cmd == AgentCancelAndRollback)
			{
				send<std::wstring>(delAgent->GetFilesToDeleteTarget(), L"");
				cont = false;
			}
			cntProcessedBefore = delAgent->GetCntProcessedFiles();
			if (deletedFiles)
			{
				(*deletedFiles)[pkgId] = *delAgent->GetDeletedFiles();
			}
			curAgent.reset();
		}
		catch(const SQLiteException& ex)
		{
			dbStmt.FreeQuery();
			delAgent->SetAgentCommand(AgentCancelAndRollback);
			send<std::wstring>(delAgent->GetFilesToDeleteTarget(), L"");
			if (IsOwnTransaction())
			{
				dbStmt.RollbackTransaction();
				dbStmt.FreeQuery();
			}
			cont = false;
			send(errBuffer, DeinstallException(pkgId, L"", ex.GetString()));
		}
		catch(...)
		{
			dbStmt.FreeQuery();
			delAgent->SetAgentCommand(AgentCancelAndRollback);
			send<std::wstring>(delAgent->GetFilesToDeleteTarget(), L"");
			if (IsOwnTransaction())
			{
				dbStmt.RollbackTransaction();
				dbStmt.FreeQuery();
			}
			cont = false;
			send(errBuffer, DeinstallException(pkgId, L"", L"unkown"));
		}
		agent::wait(delAgent.get());
	}
	done();
}

bool DeinstallAgent::IsUsingTransactions() const
{
	return usingTxFs;
}

/*------------------------------------------------------------------------*/

FilesToDeinstallGathererAgent::FilesToDeinstallGathererAgent(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete) :
	l3dPath(l3dPath), pkgIdsToDelete(pkgIdsToDelete), filesToDeinstall(make_shared<std::map<int, std::vector<FileToDeinstall>>>())
{
}

void FilesToDeinstallGathererAgent::run()
{
	AgentCommand cmd = AgentContinueWork;
	size_t cnt = 0;
	wstring filename;
	int pkgId;
	db = DBHelper::instance().GetOwnDbConnection();
	SQLiteStatement dbStmt(db.get());
	try
	{
		cntTotalFiles = 0;
		for_each(begin(pkgIdsToDelete), end(pkgIdsToDelete), [this, &dbStmt](int pkgId) {
			std::stringstream stream;
			stream << "SELECT COUNT(*) FROM Files, Dependencies WHERE Dependencies.PackageID=" << pkgId; 
			stream << " AND Dependencies.FileID=Files.ID AND Files.UsageCount == 1 AND Files.ExistedBefore == 0";
			cntTotalFiles += static_cast<size_t>(dbStmt.SqlAggregateFuncResult(stream.str()));
		});
		send<size_t>(cntProcessedFilesBuffer, 0);

		dbStmt.BeginTransaction();

		// In einer Transaktion die Dependencies löschen
		// Damit wird der Trigger ausgelöst, der Files.UsageCount verringert
		for (auto it = begin(pkgIdsToDelete); it != end(pkgIdsToDelete); ++it)
		{
			pkgId = *it;
			dbStmt.Sql("DELETE FROM Dependencies WHERE PackageID=@pkgid");
			dbStmt.BindInt(1, pkgId);
			dbStmt.ExecuteAndFree();
		}

		// Jetzt holen wir uns alle Dateien die gelöscht werden können
		std::set<int> filesToDelete;
		dbStmt.Sql("SELECT ID FROM Files WHERE UsageCount == 0 AND ExistedBefore == 0 ORDER BY Filename");
		while (dbStmt.FetchRow())
		{
			filesToDelete.insert(dbStmt.GetColumnInt(0));
		}		
		dbStmt.FreeQuery();
		// Wir machen das Löschen der Dateien in der DB rückgängig
		dbStmt.RollbackTransaction();
	
		// Jetzt fehlt noch die Zuordnung welche Datei zu welchem Package gehört
		// Dazu holen wir uns die Dateien der Packages und schauen ob sie im filesToDelete Set sind
		dbStmt.BeginTransaction();
		for (auto it = begin(pkgIdsToDelete); it != end(pkgIdsToDelete); ++it)
		{
			pkgId = *it;

			dbStmt.Sql("SELECT Filename, Files.ID FROM Files, Dependencies " 
					   "WHERE Dependencies.PackageID=@pkgId AND Dependencies.FileID=Files.ID AND Files.UsageCount <= @maxUsage AND Files.ExistedBefore == 0 ORDER BY Filename");
			dbStmt.BindInt(1, pkgId);
			dbStmt.BindInt(2, pkgIdsToDelete.size());
			while (dbStmt.FetchRow())
			{
				filename = dbStmt.GetColumnString16(0);
				int id = dbStmt.GetColumnInt(1);
				if (filesToDelete.find(id) != end(filesToDelete))
				{
					++cnt;
							
					path p = l3dPath / filename;
					WIN32_FILE_ATTRIBUTE_DATA fileAttr;
					if (GetFileAttributesEx(p.c_str(), GetFileExInfoStandard, &fileAttr))
					{
						(*filesToDeinstall)[pkgId].emplace_back(FileToDeinstall(filename, (fileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0, FileTimeToUnixTime(fileAttr.ftLastWriteTime))); 
					}

					if (cnt % 50 == 0)
					{
						send<size_t>(cntProcessedFilesBuffer, cnt);
						if (cmdBuffer.has_value())
						{
							cmd = cmdBuffer.value();
						}
						if (cmd == AgentCancelAndCommit || cmd == AgentCancelAndRollback)
						{
							break;
						}
					}
				}

			}
			dbStmt.FreeQuery();
		}
		dbStmt.RollbackTransaction();
		send<size_t>(cntProcessedFilesBuffer, cnt);
	} 
	catch(const SQLiteException& ex)
	{
		dbStmt.FreeQuery();
		dbStmt.RollbackTransaction();
		send(errBuffer, DeinstallException(pkgId, filename, ex.GetString()));
	}
	done();
}


/*------------------------------------------------------------------------*/

RemoveFilesAgent::RemoveFilesAgent(const boost::filesystem::path& l3dPath, bool logDeletedFiles, KTMTransaction* transactFs) :
	l3dPath(l3dPath), logDeletedFiles(logDeletedFiles), deletedFiles(make_shared<std::vector<std::wstring>>()), txFs(transactFs)
{
	if (txFs == nullptr)
	{
		ownTxFs = unique_ptr<KTMTransaction>(new KTMTransaction());
		txFs = ownTxFs.get();
	}
	usingTxFs = txFs->UseTransactedFunctions();
}


size_t RemoveFilesAgent::GetCntProcessedFiles()
{
	if (cntProcessedFilesBuffer.has_value())
	{
		return cntProcessedFilesBuffer.value();
	}
	return 0;
}

/**
 * Liefert zurück ob es eine eigene Transaktion ist oder ob Transaktion von außen gesteuert wird
 */
bool RemoveFilesAgent::IsOwnTransaction() const
{
	return ownTxFs || txFs == nullptr;
}

void RemoveFilesAgent::run()
{
	AgentCommand cmd = AgentContinueWork;

	wstring filename;
	size_t cnt = 0;
	do
	{
		filename = receive(filesToDeleteBuffer);
		if (filename.length() > 0)
		{
			++cnt;
			path p = l3dPath / filename;
			WIN32_FILE_ATTRIBUTE_DATA fileAttr;
			if (txFs->GetFileAttributesExW(p.c_str(), GetFileExInfoStandard, &fileAttr))
			{
				if ((fileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
				{
					txFs->SetFileAttributes(p.c_str(), fileAttr.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
				}
				if (logDeletedFiles)
				{
					deletedFiles->push_back(filename);
				}	

				if (!backupDir.empty() && !undoInstallation)
				{
					boost::filesystem::path backupPath = backupDir / filename;
					l3d::packageinstaller::fs::CreateDirectoriesTransacted(*txFs, backupPath.parent_path());
					txFs->CopyFileEx(p.c_str(), backupPath.c_str(), nullptr, nullptr, nullptr, COPY_FILE_ALLOW_DECRYPTED_DESTINATION);
				}

				if (!txFs->DeleteFileW(p.c_str()))
				{
					send(errBuffer, make_pair(filename, lhWinAPI::GetLastErrorS()));
					break;
				}

				boost::filesystem::path dirToDel = p.parent_path();
				while (dirToDel.wstring().length() > l3dPath.wstring().length() && txFs->RemoveDirectoryW(dirToDel.c_str()))
				{
					dirToDel = dirToDel.parent_path();
				}
			}
			if (cnt % 50 == 0)
			{
				send<size_t>(cntProcessedFilesBuffer, cnt);
				if (cmdBuffer.has_value())
				{
					cmd = cmdBuffer.value();
					if (cmd == AgentCancelAndCommit || cmd == AgentCancelAndRollback)
					{
						break;
					}
				}
			}
		}
	} while (filename.length() > 0);

	if (undoInstallation)
	{
		fs::MoveDirectory(*txFs, backupDir.c_str(), l3dPath.c_str());
	}

	send<size_t>(cntProcessedFilesBuffer, cnt);
	cmd = cmdBuffer.value();
	if (IsOwnTransaction())
	{
		if (errBuffer.has_value() || cmd == AgentCancelAndRollback || cmd == AgentStopAndRollback)
		{
			txFs->RollBack();
		}
		else
		{
			txFs->Commit();
		}
	}
	done();
}

bool RemoveFilesAgent::IsUsingTxFs() const
{
	return usingTxFs;
}

/*------------------------------------------------------------------------*/

DeinstallManager::DeinstallManager(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete) :
	l3dPath(l3dPath), packagesToDeinstall(pkgIdsToDelete), deinstallState(DeinstallNotRunning), undoInstallation(false)
{
}


DeinstallManager::~DeinstallManager(void)
{
	Wait();
}


bool DeinstallManager::IsUsingTransactions() const
{
	shared_ptr<ThrowableDeinstallAgent> p = agent;
	if (p)
	{
		shared_ptr<DeinstallAgent> pa = dynamic_pointer_cast<DeinstallAgent>(p);
		if (pa != nullptr)
		{
			return pa->IsUsingTransactions();
		}
	}
	return false;
}

void DeinstallManager::StartGetFilesToDelete()
{
	if (deinstallState != DeinstallNotRunning)
	{
		// Methode kann nur ein einziges Mal, ganz zu Beginn aufgerufen werden
		throw std::invalid_argument("DeinstallManager::StartGetFilesToDelete() must be called as first method in object and cannot be called afterwards");
	}
	agent = make_shared<FilesToDeinstallGathererAgent>(l3dPath, packagesToDeinstall);
	agent->start();
	deinstallState = DeinstallGatheringFiles;
}


void DeinstallManager::StartDeinstall()
{
	if (deinstallState != DeinstallNotRunning)
	{
		// Methode kann nur ein einziges Mal, ganz zu Beginn aufgerufen werden
		throw std::invalid_argument("DeinstallManager::StartDeinstall() must be called as first method in object and cannot be called afterwards");
	}
	agent = make_shared<DeinstallAgent>(l3dPath, packagesToDeinstall);
	static_cast<DeinstallAgent*>(agent.get())->SetUndoInstallation(undoInstallation);
	agent->start();
	deinstallState = DeinstallRunning;
}

void DeinstallManager::StartDeinstall(std::shared_ptr<std::map<int, std::vector<std::wstring>>> packagesAndFilesToDelete)
{
	if (deinstallState != DeinstallNotRunning && deinstallState != DeinstallGatheringFinished)
	{
		// Methode kann nur ein einziges Mal, ganz zu Beginn aufgerufen werden
		throw std::invalid_argument("DeinstallManager::StartDeinstall() can only be called when no deinstall-task is currently running");
	}
	agent = make_shared<DeinstallAgent>(l3dPath, packagesAndFilesToDelete);
	agent->start();
	deinstallState = DeinstallRunning;
}

bool DeinstallManager::IsFinished()
{
	bool ret = true;
	shared_ptr<ThrowableDeinstallAgent> p = agent;
	try
	{
		if (p)
		{
			agent_status st = p->status();
			ret = st == agent_done || st == agent_canceled;
			p->ThrowExceptionIfAvailable();
		}
		if (ret && deinstallState == DeinstallGatheringFiles)
		{
			deinstallState = DeinstallGatheringFinished;
		}
		else if (ret && deinstallState == DeinstallRunning)
		{
			deinstallState = DeinstallFinished;
		}
	} catch (const DeinstallException& ex)
	{
		deinstallState = DeinstallError;
		deinstallException.reset(new DeinstallException(ex));
	}
	return ret;
}

void DeinstallManager::Wait(unsigned int _Timeout /*= COOPERATIVE_TIMEOUT_INFINITE*/)
{
	shared_ptr<ThrowableDeinstallAgent> p = agent;
	if (p)
	{
		agent::wait(p.get(), _Timeout);
	}
}


size_t DeinstallManager::GetProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles)
{
	shared_ptr<ProgressReportableAgent> p = agent;
	if (p)
	{
		return p->GetProgress(cntProcessedFiles, cntTotalFiles);
	}
	if (cntProcessedFiles != nullptr)
	{
		cntProcessedFiles = 0;
	}
	if (cntTotalFiles != nullptr)
	{
		cntTotalFiles = 0;
	}
	return 0;
}

DeinstallManager::DeinstallState DeinstallManager::GetState()
{
	IsFinished();
	return deinstallState;
}

void DeinstallManager::CancelDeinstall()
{
	shared_ptr<ProgressReportableAgent> p = agent;
	if (p)
	{
		p->SetAgentCommand(AgentCancelAndRollback);
		deinstallState = DeinstallCancelled;
	}
}

/**
 * Bricht alle laufenden Operationen ab, und wartet bis sämtliche BG-Tasks beendet sind
 */
void DeinstallManager::CancelAndWaitForTasks()
{
	shared_ptr<ProgressReportableAgent> p = agent;
	if (p)
	{
		p->SetAgentCommand(AgentCancelAndRollback);
		deinstallState = DeinstallCancelled;
		agent::wait(p.get());
	}
}

std::shared_ptr<std::map<int, std::vector<std::wstring>>> DeinstallManager::GetDeinstalledFiles()
{
	IsFinished();
	if (deinstallState == DeinstallFinished)
	{
		shared_ptr<ProgressReportableAgent> p = agent;
		if (p)
		{
			return static_pointer_cast<DeinstallAgent>(p)->GetDeinstalledFiles();
		}
	}
	throw invalid_argument("deinstallSTate must be DeinstallFinished before calling RetrieveDeinstalledFiles");
}


std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> DeinstallManager::GetFilesToDeinstall()
{
	IsFinished();
	if (deinstallState == DeinstallGatheringFinished)
	{
		shared_ptr<ProgressReportableAgent> p = agent;
		if (p)
		{
			return static_pointer_cast<FilesToDeinstallGathererAgent>(p)->GetFilesToDeinstall();
		}
	}
	throw invalid_argument("deinstallSTate must be DeinstallGatheringFinished before calling RetrieveFilesToDeinstall");
}

}
}