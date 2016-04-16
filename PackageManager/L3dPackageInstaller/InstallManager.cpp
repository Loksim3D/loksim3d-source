#include "StdAfx.h"
#include "InstallManager.h"

#include <boost/algorithm/string.hpp>
#include <boost/thread/thread.hpp>
#include <boost/filesystem.hpp>

#include <fstream>

#include <algorithm>

#include "Zip/unzip.h"
#include "PackageFileNameUtils.h"
#include <unique_handle.h>
#include <lhWinAPI.h>
#include <lhException.h>
#include "DBHelper.h"
#include <KompexSQLiteStatement.h>
#include <KompexSQLiteException.h>

#include "FileSystemUtils.h"
#include "DeinstallManager.h"

namespace l3d
{
	namespace packageinstaller
	{
		using namespace std;
		using namespace Concurrency;
		using namespace insthelper;
		using namespace Kompex;
		using namespace l3d::packageinstaller::fs;

		InstallManager::InstallManager(const std::wstring& loksimPath) : 
			l3dPath(loksimPath), replaceOnlyOlder(true), alwaysAskBeforeOverwrite(false), installState(InstallPreparing), cntTotalFiles(0), cancelled(false), dbAgent(loksimPath), installInfoLock()
		{
			usedTransactions = transactFs.UseTransactedFunctions();
			dbAgent.SetUseTransaction(usedTransactions);
			rootBackupPath = l3dPath / L"\\Backup\\Install";
		}


		InstallManager::~InstallManager(void)
		{
			CancelAndWaitForTasks();
		}

		void InstallManager::WaitUntilFinished()
		{
			bgWork.wait();
		}

		void InstallManager::CancelAndWaitForTasks()
		{
			try
			{
				if (eventInstallFinished.wait(0) != 0)
				{
					workerTskGrp.cancel();
					bgWork.cancel();
					try
					{
						workerTskGrp.wait();
					} catch (const std::exception& ex)
					{
						// TODO log? kann zb passieren falls user auf abbrechen klickt und beim preinstallworker ex aufgetreten ist
						const char* s  = ex.what();
					}
					dbAgent.cancel();
					if (dbAgent.status() != agent_created)
					{
						agent::wait(&dbAgent);
					}
				}
			bgWork.wait();
			} catch (...)
			{
			}
		}

		void InstallManager::CancelInstallation()
		{
			//cancelled = true;
			workerTskGrp.cancel();
			//bgWork.cancel();
		}

		bool InstallManager::AddPackageToInstall(const wstring& fileName)
		{
			if (find(selectedFilesToInstall.begin(), selectedFilesToInstall.end(), fileName) == selectedFilesToInstall.end())
			{
				selectedFilesToInstall.push_back(fileName);
				PreInstallWorker *ptr = new PreInstallWorker(fileName);
				installData[fileName] = unique_ptr<PreInstallWorker>(ptr);
				workerTskGrp.run([ptr]
				{
					try
					{
						ptr->DoPreInstallWork();
						ptr->GetWorkFinishedEvent().set();
					} catch (...)
					{
						ptr->GetWorkFinishedEvent().set();
						throw;
					}
				});
				return true;
			}
			return false;
		}

		/**
		 * Entfernt ein Package aus der Liste zu installierender Packages<br>
		 * Diese Methode darf nicht mehr ausgeführt werrden, wenn die Installation bereits begonnen wurde!
		 * @param fileName Pfad zum Package
		 * @return liefert true falls package entfernt wurde; false falls es nicth in der liste vorhanden ist
		 */
		bool InstallManager::RemovePackageToInstall(const std::wstring& fileName)
		{
			auto found = find(selectedFilesToInstall.begin(), selectedFilesToInstall.end(), fileName);
			if (found != selectedFilesToInstall.end())
			{
				selectedFilesToInstall.erase(found);
				return true;
			}
			return false;
		}

		bool InstallManager::IsPreInstallFinished(const wstring& packageFileName)
		{
			auto pkgData = installData.find(packageFileName);
			if (pkgData != installData.end())
			{
				return pkgData->second->GetWorkFinishedEvent().wait(0) == 0;
			}
			return false;
		}

		const wstring& InstallManager::GetReadmeForPackage(const wstring& packageFileName)
		{
			return installData[packageFileName]->GetReadme();
		}

		bool isReadmeDeleteOrPackFile(const wstring& name)
		{
			return boost::iequals(name, L"readme.txt") || boost::iequals(name, L"DeleteFiles.txt") || boost::iends_with(name, L".l3dpack") ||
				boost::iequals(name, L"PackageInfo.xml");
		}

		void InstallManager::ThrowZipException(ZRESULT zipRes, const PackageInfo& pkgInfo, const char *exMsg, const std::wstring& curFile)
		{
			wchar_t buf[256];
			FormatZipMessage(ZR_RECENT, buf, sizeof buf / sizeof buf[0]);
			InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorZipOccurred, pkgInfo, buf, curFile);
			throw InstallerException(errInfo, exMsg);
		}

		void InstallManager::BackupFile(const boost::filesystem::path& l3dRelativeFilePath, insthelper::InstallInformation* installInfo)
		{
			boost::filesystem::path curPath = l3dPath / l3dRelativeFilePath;
			boost::filesystem::path pkgPath = installInfo->packageName;
			boost::filesystem::path newPath = currentBackupPath / l3dRelativeFilePath;
			if (!FileExistsTransacted(transactFs, newPath.c_str()))
			{
				CreateDirectoriesTransacted(transactFs, newPath.parent_path());
				transactFs.CopyFileEx(curPath.c_str(), newPath.c_str(), nullptr, nullptr, nullptr, COPY_FILE_ALLOW_DECRYPTED_DESTINATION);
			}
		}

		bool InstallManager::IsInDeleteList(const wstring& fname, const insthelper::InstallInformation& installInfo)
		{
			boost::filesystem::path bpath = fname;
			if (!fname.empty() && fname[0] != '/' && fname[0] != '\\')
			{
				bpath = L"/" / bpath;
			}
			bpath = bpath.make_preferred();
			auto func = [&bpath](const insthelper::FileInstallInfo& inf) -> bool
			{
				auto p = boost::filesystem::path(inf.fileName).make_preferred();
				return bpath == p;
			};
			
			bool found = find_if(begin(installInfo.filesToDelete), end(installInfo.filesToDelete), func) != end(installInfo.filesToDelete);
			return found || find_if(begin(installInfo.writeProtectedFilesToDelete), end(installInfo.writeProtectedFilesToDelete), func) != end(installInfo.writeProtectedFilesToDelete);
		}

		bool InstallManager::IsInDeinstallList(const std::wstring& fname)
		{
			if (!filesToDeinstall)
			{
				return false;
			}
			boost::filesystem::path bpath = fname;
			if (!fname.empty() && fname[0] != '/' && fname[0] != '\\')
			{
				bpath = L"/" / bpath;
			}
			bpath = bpath.make_preferred();
			return find_if(begin(*filesToDeinstall), begin(*filesToDeinstall), [&bpath](const pair<int, std::vector<FileToDeinstall>>& en) -> bool
			{
				const auto& bp = bpath;
				return find_if(begin(en.second), end(en.second), [&bp](const FileToDeinstall& ftd) -> bool
				{
					auto p = boost::filesystem::path(ftd.filename).make_preferred();
					return bp == p;
				}) != end(en.second);
			})  != end(*filesToDeinstall);
		}

		void InstallManager::InstallFiles(size_t startIndex, size_t end, const RootPackageInfo& rootPkg, const PackageInfo& pkgInfo, insthelper::InstallInformation* installInfo)
		{
			size_t zipBufSize = 131072;
			unique_ptr<char[]> zipBuf(new char[zipBufSize]);

			const size_t fileBufSize = 16384;
			unique_ptr<char[]> fileBuf(new char[fileBufSize]);

			ZRESULT zipRes;

			UserSelection *userInstSel = nullptr;
			if (installState == InstallPassTwoRunning)
			{
				userInstSel = &(*userSelectedFiles)[installInfo->packageName];
			}

			unique_handle<HZIP, hzip_traits> hZip = unique_handle<HZIP, hzip_traits>(OpenZip(pkgInfo.packageName.c_str(), nullptr));
			if (!hZip)
			{
				ThrowZipException(ZR_RECENT, pkgInfo, "Error opening zip file", pkgInfo.packageName);
			}

			ZIPENTRY ze; 
			size_t processed = 0;
			for (size_t i = startIndex; i < end && !workerTskGrp.is_canceling(); ++i)
			{
				zipRes = GetZipItem(hZip.get(), i, &ze);
				if (zipRes != ZR_OK)
				{
					ThrowZipException(zipRes, pkgInfo, "Error calling GetZipItem", pkgInfo.packageName);
				}
				std::wstring fname = PackageFileNameToFileName(ze.name);

				if ((ze.attr & FILE_ATTRIBUTE_DIRECTORY) != 0 || isReadmeDeleteOrPackFile(fname))
				{
					continue;
				}

				boost::filesystem::path diskPath = l3dPath;
				diskPath /= fname;
				bool fileExists = FileExistsTransacted(transactFs, diskPath.c_str());

				WIN32_FILE_ATTRIBUTE_DATA diskFileAttr = { 0 };
				if (fileExists) 
				{
					transactFs.GetFileAttributesExW(diskPath.c_str(), GetFileExInfoStandard, &diskFileAttr);
				}

				++processed;
				if (processed > 30)
				{
					InterlockedExchangeAdd(&installInfo->cntProcessedFiles, processed);
					processed = 0;
				}

				if (userInstSel != nullptr && !binary_search(userInstSel->filesToInstall.begin(), userInstSel->filesToInstall.end(), fname)) 
				{
					// wir befinden uns im pass two und der user hat unsere datei nicht ausgewählt => continue
					if (fileExists)
					{
						send(dbAgent.GetFileInfoTarget(), make_shared<db::DbInstallFileInfo>(FileInstallInfo(fname, ze.mtime, diskFileAttr.ftLastWriteTime), false)); 
					}
					else
					{
						send(dbAgent.GetFileInfoTarget(), make_shared<db::DbInstallFileInfo>(FileInstallInfo(fname, ze.mtime), false)); 
					}
					continue;
				}
				
				bool installFile = true;
				if (fileExists && userInstSel == nullptr && !IsInDeleteList(fname, *installInfo))
				{
					// Falls nur ältere Dateien ersetzen werden sollen, Modified Times von Disk und Zip prüfen
					// TODO Bug bei Deinstallation - auch neue Dateien werden ueberschrieben
					if(replaceOnlyOlder /*&& !IsInDeinstallList(fname)*/)
					{
						FILETIME diskTime = diskFileAttr.ftLastWriteTime;
						FILETIME zipTime = ze.mtime;
						installFile = CompareFileTime(&diskTime, &zipTime) < 0;
					}
					if (installFile)
					{
						if (static_cast<size_t>(ze.unc_size) > zipBufSize)
						{
							zipBufSize = max(131072, ze.unc_size);
							zipBuf.reset(new char[zipBufSize]);
						}
						zipRes = UnzipItem(hZip.get(), i, zipBuf.get(), zipBufSize);

						if ((diskFileAttr.nFileSizeHigh << sizeof (DWORD) | diskFileAttr.nFileSizeLow) == ze.unc_size)
						{
							// Falls filesize gleich, müssen wir den Inhalt vergleichen
							handle h (transactFs.CreateFile(diskPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
							if (h.get() == INVALID_HANDLE_VALUE)
							{
								InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, pkgInfo, lhWinAPI::GetLastErrorS(),
									diskPath.wstring());
								throw InstallerException(errInfo, "Error open file while file compare");
							}

							installFile = false;
							DWORD bytesRead;
							size_t overallRead = 0;
							do 
							{
								if (ReadFile(h.get(), fileBuf.get(), fileBufSize, &bytesRead, nullptr))
								{
									if (overallRead + bytesRead > static_cast<size_t>(ze.unc_size) || memcmp(zipBuf.get() + overallRead, fileBuf.get(), bytesRead) != 0)
									{
										overallRead += bytesRead;
										installFile = true;
										break;
									}
									overallRead += bytesRead;
								}
								else
								{
									InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, pkgInfo, lhWinAPI::GetLastErrorS(),
										diskPath.wstring());
									throw InstallerException(errInfo, "Error reading file while file compare");
								}
							} while (bytesRead > 0);
							if (!installFile)
							{
								// prüfen ob gelesene anzahl bytes != bytes in zipfile sind
								installFile = overallRead != ze.unc_size;
							}
						}
					}
				} 
				else
				{
					if (static_cast<size_t>(ze.unc_size) > zipBufSize)
					{
						zipBufSize = max(131072, ze.unc_size);
						zipBuf.reset(new char[zipBufSize]);
					}
					zipRes = UnzipItem(hZip.get(), i, zipBuf.get(), zipBufSize);
					if (zipRes != ZR_OK)
					{
						ThrowZipException(zipRes, pkgInfo, "Error calling GetZipItem", pkgInfo.packageName);
					}
				}
				if (installFile)
				{		
					if (userInstSel == nullptr)
					{
						if (fileExists && (diskFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
						{
							// File is write protected
							installInfo->writeProtectedFilesToInstall.push_back(FileInstallInfo(fname, ze.mtime, diskFileAttr.ftLastWriteTime));
						}
						else if (fileExists)
						{
							// File existiert, ist aber nicht schreibgeschützt
							installInfo->filesToInstall.push_back(FileInstallInfo(fname, ze.mtime, diskFileAttr.ftLastWriteTime));
						}
						else
						{
							// File existiert nicht auf Disk
							installInfo->filesToInstall.push_back(FileInstallInfo(fname, ze.mtime));
						}

						if (alwaysAskBeforeOverwrite)
						{
							// Benutzer will für jede Datei explizit gefragt werden =>
							// nur in Liste aufnehmen
							continue;
						}
					}

					if (!FileExistsTransacted(transactFs, diskPath.parent_path().c_str()))
					{
						if (!CreateDirectoriesTransacted(transactFs, diskPath.parent_path()) && !FileExistsTransacted(transactFs, diskPath.parent_path().c_str()))
						{
							InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, pkgInfo, lhWinAPI::GetLastErrorS(), diskPath.wstring());
							throw InstallerException(errInfo, "Error create directory");
						}
					}

					if (fileExists && (diskFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
					{
						// Schreibgeschützt => Schreibschutz aufheben
						transactFs.SetFileAttributes(diskPath.c_str(), diskFileAttr.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
					}

					if (fileExists)
					{
						BackupFile(fname, installInfo);
						send(dbAgent.GetFileInfoTarget(), make_shared<db::DbInstallFileInfo>(FileInstallInfo(fname, ze.mtime, diskFileAttr.ftLastWriteTime), true)); 
					}
					else
					{
						send(dbAgent.GetFileInfoTarget(), make_shared<db::DbInstallFileInfo>(FileInstallInfo(fname, ze.mtime), true)); 
					}

					handle h (transactFs.CreateFileW(diskPath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
					if (h.get() != INVALID_HANDLE_VALUE)
					{
						DWORD bytesWritten;
						if (!WriteFile(h.get(), zipBuf.get(), ze.unc_size, &bytesWritten, nullptr))
						{
							InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, pkgInfo, lhWinAPI::GetLastErrorS(), diskPath.wstring());
							throw InstallerException(errInfo, "Error writing output file");
						}
						SetFileTime(h.get(), nullptr, nullptr, &ze.mtime);
						if (fileExists && (diskFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
						{
							// Datei war schreibgeschützt != Schreibschutz wieder setzen
							transactFs.SetFileAttributes(diskPath.c_str(), diskFileAttr.dwFileAttributes);
						}
					}
					else
					{
						InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, pkgInfo, lhWinAPI::GetLastErrorS(), diskPath.wstring());
						throw InstallerException(errInfo, "Error creating output file");
					}
				}
				else if (!alwaysAskBeforeOverwrite || installState == InstallPassTwoRunning)
				{
					if (fileExists)
					{
						send(dbAgent.GetFileInfoTarget(), make_shared<db::DbInstallFileInfo>(FileInstallInfo(fname, ze.mtime, diskFileAttr.ftLastWriteTime), false)); 
					}
					else
					{
						send(dbAgent.GetFileInfoTarget(), make_shared<db::DbInstallFileInfo>(FileInstallInfo(fname, ze.mtime), false)); 
					}
				}
			}
			InterlockedExchangeAdd(&installInfo->cntProcessedFiles, processed);
		}

		void InstallManager::InstallPackage(size_t numThreads, const RootPackageInfo& rootPkg, const PackageInfo& pkgInfo, InstallInformation* instInfo)
		{
			for (const auto& instData : installData) {
				auto itPkg = instData.second->GetPackagGuidsToDeinstall().find(L"@" + rootPkg.hash);
				if (itPkg != instData.second->GetPackagGuidsToDeinstall().end()) {
					// Package wuerde deinstalliert werden
					// => bei Installation ignorieren
					return;
				}
			}

			size_t itemsPerThread = pkgInfo.numElements / numThreads;
			for (size_t t = 0; t < numThreads; ++t)
			{
				size_t startIndex = t * itemsPerThread;
				size_t end = startIndex + itemsPerThread;
				if (t == numThreads - 1)
				{
					end = pkgInfo.numElements;
				}
				workerTskGrp.run([this, pkgInfo, startIndex, end, instInfo, rootPkg] () {
					InstallFiles(startIndex, end, rootPkg, pkgInfo, instInfo);
				});
			}
		}

		void InstallManager::DeleteFiles(const vector<wstring>& filesToDelete, InstallInformation* instInfo, const PackageInfo& pkgInfo)
		{
			UserSelection *userInstSel = nullptr;
			if (installState == InstallPassTwoRunning)
			{
				userInstSel = &(*userSelectedFiles)[instInfo->packageName];
			}
			for_each(filesToDelete.begin(), filesToDelete.end(), [this, &instInfo, &pkgInfo, userInstSel] (const wstring& fname) {
				boost::filesystem::path diskPath = l3dPath;
				diskPath /= fname;
				WIN32_FILE_ATTRIBUTE_DATA diskFileAttr = { 0 };

				if (transactFs.GetFileAttributesExW(diskPath.c_str(), GetFileExInfoStandard, &diskFileAttr))
				{
					if (installState != InstallPassTwoRunning)
					{
						if ((diskFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
						{
							instInfo->writeProtectedFilesToDelete.push_back(FileInstallInfo(fname, diskFileAttr.ftLastWriteTime, true));
						}
						else
						{
							instInfo->filesToDelete.push_back(FileInstallInfo(fname, diskFileAttr.ftLastWriteTime, true));
						}
					}

					if ((!alwaysAskBeforeOverwrite || installState == InstallPassTwoRunning) && 
						(userInstSel == nullptr || userInstSel->filesToDelete.find(fname) != userInstSel->filesToDelete.end()))
					{
						BackupFile(fname, instInfo);
						if ((diskFileAttr.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0)
						{
							transactFs.SetFileAttributes(diskPath.c_str(), diskFileAttr.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
						}
						if ((diskFileAttr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
						{
							// Funktion entfernt nur leere Verzeichnisse => das wollen wir auch genauso
							transactFs.RemoveDirectory(diskPath.c_str());
						}
						else if (!transactFs.DeleteFileW(diskPath.c_str()))
						{
							InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, pkgInfo, lhWinAPI::GetLastErrorS(), diskPath.wstring());
							throw InstallerException(errInfo, "Error deleting file");
						}
					}
				}
			});

		}

		size_t InstallManager::GetInstallationProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles)
		{
			if (cntTotalFiles != nullptr)
			{
				*cntTotalFiles = this->cntTotalFiles;
			}
			size_t processed = 0;
			if (!alwaysAskBeforeOverwrite || installState == InstallPassTwoRunning)
			{
				try_receive<size_t>(dbAgent.GetCntProcessedFilesSource(), processed);
			}
			else
			{
				reader_writer_lock::scoped_lock_read lck(installInfoLock);
				for (auto instInfo = installInformation.begin(); instInfo != installInformation.end(); ++instInfo)
				{
					processed += instInfo->second->cntProcessedFiles;
				}
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

		/**
		 * Startet zweiten Durchgang der Installation<br>
		 * Installiert / Löscht alle Dateien die im übergebenen Argument gesetzt sind<br>
		 * Setzt das EventInstallFinished zurück. Dieses Event kann nun wieder verwendet werden, um auf den Abschluss der Installation zu warten
		 * Aufruf nur gültig falls IsAlwaysAsk() true liefert
		 * @param userSelection Auswahl des Benutzers welche Dateien in welchem Package zu installieren sind
		 */
		void InstallManager::StartInstallUserSelection(std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> userSelection)
		{				
			eventInstallFinished.reset();
			this->userSelectedFiles = userSelection;
			installState = InstallRunning;

			{
				reader_writer_lock::scoped_lock_read lck(installInfoLock);
				for (auto it = installInformation.begin(); it != installInformation.end(); ++it)
				{
					it->second->cntProcessedFiles = 0;
				}
			}

			dbAgent.start();

			InstallManager *mt = this;
			auto& outerSelFiles = selectedFilesToInstall;
			bgWork.run([mt, outerSelFiles] {
				// VS2010 SP1 Bug nested lambda Connect ID 537366 
				InstallManager *m = mt;
				auto& selFiles = outerSelFiles;
				mt->DoExceptionHandledInstall([m, &selFiles]() {
					m->cancelled = m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled;

					m->installState = InstallManager::InstallPassTwoRunning;

					//TODO
					vector<int> deinstallPkgIds = m->DetermineFilesToDeinstall();

					// einen core explizit für db operationen "reservieren"
					size_t numThreads = max(1, boost::thread::hardware_concurrency() - 1);
					for (auto fileName = selFiles.begin(); fileName != selFiles.end() && !m->workerTskGrp.is_canceling(); ++fileName)
					{
						//UserSelection& uEntry = (*m->userSelectedFiles)[*fileName];
						InstallInformation *instInfo = nullptr;
						{
							reader_writer_lock::scoped_lock_read lck(m->installInfoLock);
							instInfo = m->installInformation[*fileName].get();
						}
						unique_ptr<PreInstallWorker>& refData = m->installData.at(*fileName);

						m->SetBackupDir(refData->GetPackageInfo());
						send(m->dbAgent.GetPkgBeginTarget(), &refData->GetPackageInfo());
						refData->GetPackageInfo().dbId = receive(m->dbAgent.GetPkgDbIdSource()); 
						m->dbAgent.ThrowExceptionIfAvailable();

						m->DeleteFiles(refData->GetFilesToDelete(), instInfo, refData->GetPackageInfo());

						m->InstallPackage(numThreads, refData->GetPackageInfo(), refData->GetPackageInfo(), instInfo);
						m->cancelled =  m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled;

						const std::vector<PackageInfo>& innerPackages = refData->GetContainedPackages();
						for (auto innerPkgs = innerPackages.begin(); innerPkgs != innerPackages.end() && !m->workerTskGrp.is_canceling(); ++innerPkgs)
						{
							m->InstallPackage(numThreads, refData->GetPackageInfo(), *innerPkgs, instInfo);
							m->cancelled = m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled;
						}
						// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
						send(m->dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					}

					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(m->dbAgent.GetPkgBeginTarget(), nullptr);

					m->cancelled = m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled || m->workerTskGrp.is_canceling();
					if (m->cancelled)
					{
						m->installState = InstallManager::InstallCancelled;
						if (m->transactFs.RollBack() && m->IsUsingTransaction())
						{
							m->dbAgent.SetAgentCommand(AgentCancelAndRollback);
						}
	
						// Falls filesystem rollback fehlschlägt, speichern wir die installation in der db
						// damit eine deinstallation möglich ist
						m->dbAgent.SetAgentCommand(AgentCancelAndCommit);
						agent::wait(&m->dbAgent);
						m->dbAgent.ThrowExceptionIfAvailable();
					}
					else
					{
						if (m->filesToDeinstall)
						{
							DeinstallAgent da(m->l3dPath, deinstallPkgIds, m->dbAgent.GetConnection(), &m->transactFs);
							da.start();
							agent::wait(&da);
							da.ThrowExceptionIfAvailable();
							m->deinstalledFiles = da.GetDeinstalledFiles();
						}

						m->dbAgent.SetAgentCommand(AgentStopAndCommit);
						agent::wait(&m->dbAgent);
						m->dbAgent.ThrowExceptionIfAvailable();

						if (!m->transactFs.Commit())
						{
							InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, PackageInfo(), lhWinAPI::GetLastErrorS());
							throw InstallerException(errInfo, "Transaction commit failed");
						}
						else
						{
							m->installState = InstallManager::InstallSuccessfullyFinished;
						}
					}
				});
			});
		}

		void InstallManager::SetBackupDir(RootPackageInfo& pkgInfo)
		{
			boost::filesystem::path pkgPath = pkgInfo.packageName;
			currentBackupPath = rootBackupPath / pkgPath.stem();
			int nr = 1;
			while (FileExistsTransacted(transactFs, currentBackupPath.c_str()))
			{
				currentBackupPath = rootBackupPath / (pkgPath.stem().wstring() + lhstd::String(nr));
				nr++;
			}
			pkgInfo.backupDirectory = currentBackupPath.wstring();
		}

		vector<int> InstallManager::DetermineFilesToDeinstall()
		{
			vector<int> deinstallPkgIds;

			if (installState == InstallPassTwoRunning)
			{
				for(auto& en = begin(*userSelectedFiles); en != end(*userSelectedFiles); ++en)
				{
					copy(begin(en->second.packagesToDeinstall), end(en->second.packagesToDeinstall), back_inserter(deinstallPkgIds));
				}
			}
			else
			{
				packagesToDeinstall = make_shared<map<int, db::DBPackageEntry>>();
				for(auto& en = begin(installData); en != end(installData); ++en)
				{
					std::transform(begin(en->second->GetPackagesToDeinstall()), end(en->second->GetPackagesToDeinstall()), back_inserter(deinstallPkgIds), [](const db::DBPackageEntry& dbEn)
					{
						return dbEn.GetId();
					});
				
					std::for_each(begin(en->second->GetPackagesToDeinstall()), end(en->second->GetPackagesToDeinstall()), 
						[this](const db::DBPackageEntry& dbEn)
					{
						(*packagesToDeinstall)[dbEn.GetId()] = dbEn;
					});
				}
			}
			if (!deinstallPkgIds.empty())
			{
				FilesToDeinstallGathererAgent ga(l3dPath, deinstallPkgIds);
				ga.start();
				agent::wait(&ga);
				ga.ThrowExceptionIfAvailable();
				filesToDeinstall = ga.GetFilesToDeinstall();
			}
			else
			{
				filesToDeinstall = nullptr;
			}
			return deinstallPkgIds;
		}

		void InstallManager::StartInstallation()
		{		
			if (alwaysAskBeforeOverwrite && selectedFilesToInstall.size() > 1)
			{
				throw std::invalid_argument("alwaysAskBeforeOverwrite only possible if installing one file");
			}

			InstallManager* mt = this;
			installState = InstallRunning;

			{
				wofstream outStr(GetFilenameInLocalSettingsDir(FILENAME_CRASHRPT_LOG), fstream::out | fstream::app);
				outStr << L"Files To Install:" << endl;
				std::for_each(begin(selectedFilesToInstall), end(selectedFilesToInstall), [&outStr](const wstring& f) {
					outStr << f << endl;
				});
			}
			
			if (!alwaysAskBeforeOverwrite)
			{
				dbAgent.start();
			}
			auto& outerSelFiles = selectedFilesToInstall;
			bgWork.run([mt, outerSelFiles] {
				// VS2010 SP1 Bug nested lambda Connect ID 537366 
				InstallManager *m = mt;
				auto& selFiles = outerSelFiles;
				mt->DoExceptionHandledInstall([m, &selFiles]() {
					m->cancelled = m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled;

					m->cntTotalFiles = 0;
					for (auto fileName = selFiles.begin(); fileName != selFiles.end() && !m->workerTskGrp.is_canceling(); ++fileName)
					{
						PreInstallWorker *preInstallData = m->installData.at(*fileName).get();
						m->cntTotalFiles += preInstallData->GetPackageInfo().numFilesToInstall;
						const std::vector<PackageInfo>& innerPackages = preInstallData->GetContainedPackages();
						for (auto innerPkgs = innerPackages.begin(); innerPkgs != innerPackages.end() && !m->workerTskGrp.is_canceling(); ++innerPkgs)
						{
							m->cntTotalFiles += innerPkgs->numFilesToInstall;
						}
					}

					vector<int> deinstallPkgIds = m->DetermineFilesToDeinstall();

					// einen core explizit für db operationen "reservieren"
					// außer wir machen in diesem pass nix mit der db
					size_t numThreads = max(1, boost::thread::hardware_concurrency() - (m->alwaysAskBeforeOverwrite ? 0 : 1));
					for (auto fileName = selFiles.begin(); fileName != selFiles.end() && !m->workerTskGrp.is_canceling(); ++fileName)
					{
						unique_ptr<InstallInformation> instInf = unique_ptr<InstallInformation>(new InstallInformation());
						instInf->packageName = *fileName;
						instInf->cntProcessedFiles = 0;
						InstallInformation *pInf = instInf.get();
						{
							reader_writer_lock::scoped_lock lck(m->installInfoLock);
							m->installInformation[*fileName] = move(instInf);
						}
						unique_ptr<PreInstallWorker>& refData = m->installData.at(*fileName);				

						if (!m->alwaysAskBeforeOverwrite)
						{
							m->SetBackupDir(refData->GetPackageInfo());
							send(m->dbAgent.GetPkgBeginTarget(), &refData->GetPackageInfo());
							refData->GetPackageInfo().dbId = receive(m->dbAgent.GetPkgDbIdSource()); 
							m->dbAgent.ThrowExceptionIfAvailable();
						}
					
						m->DeleteFiles(refData->GetFilesToDelete(), pInf, refData->GetPackageInfo());

						m->InstallPackage(numThreads, refData->GetPackageInfo(), refData->GetPackageInfo(), pInf);					
						m->cancelled =  m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled;

						const std::vector<PackageInfo>& innerPackages = refData->GetContainedPackages();
						for (auto innerPkgs = innerPackages.begin(); innerPkgs != innerPackages.end() && !m->workerTskGrp.is_canceling(); ++innerPkgs)
						{
							m->InstallPackage(numThreads, refData->GetPackageInfo(), *innerPkgs, pInf);
							m->cancelled = m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled;
						}
						if (!m->alwaysAskBeforeOverwrite)
						{
							// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
							send(m->dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
						}
						if (!m->cancelled)
						{
							// Sorieren für den User, bzw in weiterer folge auch wichtig für die userSelection
							sort(pInf->filesToDelete.begin(), pInf->filesToDelete.end());
							sort(pInf->filesToInstall.begin(), pInf->filesToInstall.end());
							sort(pInf->writeProtectedFilesToDelete.begin(), pInf->writeProtectedFilesToDelete.end());
							sort(pInf->writeProtectedFilesToInstall.begin(), pInf->writeProtectedFilesToInstall.end());
						}
					}

					if (!m->alwaysAskBeforeOverwrite)
					{
						// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
						send<RootPackageInfo*>(m->dbAgent.GetPkgBeginTarget(), nullptr);
					}

					m->cancelled = m->workerTskGrp.wait() == Concurrency::canceled || m->cancelled || m->workerTskGrp.is_canceling();
					if (m->cancelled)
					{
						m->installState = InstallManager::InstallCancelled;
						if (m->transactFs.RollBack() && m->IsUsingTransaction() && !m->alwaysAskBeforeOverwrite)
						{
							m->dbAgent.SetAgentCommand(AgentCancelAndRollback);
						} else if (!m->alwaysAskBeforeOverwrite)
						{
							// Falls filesystem rollback fehlschlägt, speichern wir die installation in der db
							// damit eine deinstallation möglich ist
							m->dbAgent.SetAgentCommand(AgentCancelAndCommit);
							agent::wait(&m->dbAgent);
							m->dbAgent.ThrowExceptionIfAvailable();
						}
					}
					else if (m->alwaysAskBeforeOverwrite)
					{
						m->installState = InstallManager::InstallPassOneFinished;
						m->GenerateFilesToDeinstallList(deinstallPkgIds);
					}
					else
					{
						if (m->filesToDeinstall)
						{
							DeinstallAgent da(m->l3dPath, deinstallPkgIds, m->dbAgent.GetConnection(), &m->transactFs);
							da.start();
							agent::wait(&da);
							da.ThrowExceptionIfAvailable();
							m->deinstalledFiles = da.GetDeinstalledFiles();
						}

						m->dbAgent.SetAgentCommand(AgentStopAndCommit);
						agent::wait(&m->dbAgent);
						m->dbAgent.ThrowExceptionIfAvailable();

						if (!m->transactFs.Commit())
						{
							InstallerErrorInformation errInfo(InstallerErrorInformation::InstallErrorFileSystemOccurred, PackageInfo(), lhWinAPI::GetLastErrorS());
							throw InstallerException(errInfo, "Transaction commit failed");
						}
						else
						{
							m->installState = InstallManager::InstallSuccessfullyFinished;
						}
					}
				});
				if (!mt->alwaysAskBeforeOverwrite)
				{
					agent::wait(&mt->dbAgent);
				}
			});
		}

		template<class Func>
		void InstallManager::DoExceptionHandledInstall(Func f)
		{
			try
			{
				f();
			}
			catch (const InstallerException& ex)
			{
				installState = InstallErrorOccurred;
				errorEx.reset(new InstallerException(ex));
				if (transactFs.RollBack() && IsUsingTransaction() && dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndRollback);
				}
				else if (dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndCommit);
				}
				if (dbAgent.status() != Concurrency::agent_created)
				{
					// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
					send(dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(dbAgent.GetPkgBeginTarget(), nullptr);
				}
			}
			catch (const DeinstallException& ex)
			{
				installState = InstallErrorOccurred;
				errorEx.reset(new InstallerException(ex));
				if (transactFs.RollBack() && IsUsingTransaction() && dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndRollback);
				}
				else if (dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndCommit);
				}
				if (dbAgent.status() != Concurrency::agent_created)
				{
					// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
					send(dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(dbAgent.GetPkgBeginTarget(), nullptr);
				}
			}
			catch (const SQLiteException& ex)
			{
				installState = InstallErrorOccurred;
				std::wstring msg;
				lhWinAPI::CharToTchar(msg, ex.GetString());
				errorEx.reset(new InstallerException(InstallerErrorInformation(InstallerErrorInformation::InstallErrorFileSystemOccurred, PackageInfo(), msg)));
				if (transactFs.RollBack() && IsUsingTransaction() && dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndRollback);
				}
				else if (dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndCommit);
				}
				if (dbAgent.status() != Concurrency::agent_created)
				{
					// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
					send(dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(dbAgent.GetPkgBeginTarget(), nullptr);
				}
			}
			catch (const lhstd::exception& ex)
			{
				installState = InstallErrorOccurred;
				errorEx.reset(new InstallerException(InstallerErrorInformation(InstallerErrorInformation::InstallErrorFileSystemOccurred, PackageInfo(), ex.what())));
				if (transactFs.RollBack() && IsUsingTransaction() && dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndRollback);
				}
				else if (dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndCommit);
				}
				if (dbAgent.status() != Concurrency::agent_created)
				{
					// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
					send(dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(dbAgent.GetPkgBeginTarget(), nullptr);
				}
			}
			catch (const std::exception& ex)
			{
				installState = InstallErrorOccurred;
				wstring msg;
				lhWinAPI::CharToTchar(msg, ex.what());
				errorEx.reset(new InstallerException(InstallerErrorInformation(InstallerErrorInformation::InstallErrorFileSystemOccurred, PackageInfo(), wstring(L"std::exception ").append(msg))));
				if (transactFs.RollBack() && IsUsingTransaction() && dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndRollback);
				}
				else if (dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndCommit);
				}
				if (dbAgent.status() != Concurrency::agent_created)
				{
					// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
					send(dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(dbAgent.GetPkgBeginTarget(), nullptr);
				}
			}
			catch (...)
			{
				installState = InstallErrorOccurred;
				if (transactFs.RollBack() && IsUsingTransaction() && dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndRollback);
				}
				else if (dbAgent.status() != Concurrency::agent_created)
				{
					dbAgent.SetAgentCommand(AgentStopAndCommit);
				}
				if (dbAgent.status() != Concurrency::agent_created)
				{
					// Sende 'Code' (leerer shared_ptr), dass Package fertig abgearbeitet wurde
					send(dbAgent.GetFileInfoTarget(), shared_ptr<db::DbInstallFileInfo>());
					// Sende 'Code', dass es keine Packages mehr zum Abarbeiten gibt
					send<RootPackageInfo*>(dbAgent.GetPkgBeginTarget(), nullptr);
				}
			}
			eventInstallFinished.set();
		}

		std::shared_ptr<std::vector<pair<db::DBPackageEntry, std::vector<std::wstring>>>> InstallManager::GetDeinstalledFiles() const
		{
			std::shared_ptr<std::vector<pair<db::DBPackageEntry, std::vector<std::wstring>>>> ret;
			if (packagesToDeinstall && deinstalledFiles && !deinstalledFiles->empty())
			{
				ret = make_shared<std::vector<pair<db::DBPackageEntry, std::vector<std::wstring>>>>();
				for_each(begin(*deinstalledFiles), end(*deinstalledFiles), [&ret, this](const pair<int, std::vector<std::wstring>>& en)
				{
					ret->push_back(make_pair((*packagesToDeinstall)[en.first], en.second));
				});
			}
			return ret;
		}

		// Prüft ob eine Datei in installInformation drinnen ist
		bool InstallManager::IsInInstallList(const std::wstring& fname) const
		{
			boost::filesystem::path bpath = boost::to_lower_copy(fname);
			if (!fname.empty() && fname[0] != '/' && fname[0] != '\\')
			{
				bpath = L"/" / bpath;
			}
			bpath = bpath.make_preferred();
			for (auto en = begin(installInformation); en != end(installInformation); ++en)
			{
				const auto& bp = bpath;
				bool found = find_if(begin(en->second->filesToInstall), end(en->second->filesToInstall), [&bp](const FileInstallInfo& ftd) -> bool
				{
					auto p = boost::filesystem::path(boost::to_lower_copy(ftd.fileName)).make_preferred();
					return bp == p;
				}) != end(en->second->filesToInstall);

				found = found ||  find_if(begin(en->second->writeProtectedFilesToInstall), end(en->second->writeProtectedFilesToInstall), [&bp](const FileInstallInfo& ftd) -> bool
				{
					auto p = boost::filesystem::path(boost::to_lower_copy(ftd.fileName)).make_preferred();
					return bp == p;
				}) != end(en->second->writeProtectedFilesToInstall);

				if (found)
				{
					return true;
				}
			}
			return false;
		}

		// Erzeugt Liste zu deinstallierender Dateien zur Anzeige für User
		void InstallManager::GenerateFilesToDeinstallList(vector<int> deinstallPkgIds)
		{
			userFilesToDeinstall = make_shared<vector<pair<db::DBPackageEntry, vector<wstring>>>>();

			std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> files;
			if (filesToDeinstall)
			{
				FilesToDeinstallGathererAgent da(l3dPath, deinstallPkgIds);
				da.start();
				agent::wait(&da);
				da.ThrowExceptionIfAvailable();
				files = da.GetFilesToDeinstall();
			}

			if (files && !files->empty() && packagesToDeinstall)
			{
				for_each(begin(*files), end(*files), [this](const pair<int, vector<FileToDeinstall>>& en)
				{
					auto newEl = make_pair((*packagesToDeinstall)[en.first], vector<wstring>());
					InstallManager *m = this;
					for_each(begin(en.second), end(en.second), [&m, &newEl](const FileToDeinstall& f)
					{
						if (!m->IsInInstallList(f.filename))
						{
							newEl.second.push_back(f.filename);
						}
					});
					userFilesToDeinstall->push_back(move(newEl));
				});
			}
		}

		// Liste von Packages mit einem Pair<PackageName, Name zu deinstallierender Dateien>
		shared_ptr<vector<pair<db::DBPackageEntry, vector<wstring>>>> InstallManager::GetFilesToDeinstall() const
		{
			return userFilesToDeinstall;
		}
	}
}