#pragma once

#include <Windows.h>

#include <vector>
#include <string>
#include <map>
#include <utility>
#include <memory>
#include <exception>

#include <boost/filesystem.hpp>
#include <boost/atomic.hpp>

#include <ppl.h>
#include <concurrent_vector.h>

#include <KompexSQLiteDatabase.h>
#include <KompexSQLiteStatement.h>
#include "Zip/unzip.h"
#include "KTM.h"
#include "PreInstallWorker.h"
#include "InstallerException.h"
#include "InstallHelperStructs.h"

#include "FileToDeinstall.h"

#include "InstallPkgDBAgent.h"

namespace l3d
{
namespace packageinstaller
{


/**
 * Class that handles installation of one or more packages
 */
class InstallManager
{
public:
	enum InstallManagerState
	{
		InstallPreparing, InstallRunning, InstallPassOneFinished, InstallPassTwoRunning, InstallSuccessfullyFinished, InstallErrorOccurred, InstallCancelled
	};
public:
	InstallManager(const std::wstring& loksimPath);
	~InstallManager(void);

	/**
	 * Wartet bis die Installation abgeschossen ist
	 */
	void WaitUntilFinished();

	/**
	 * Fügt Package zum Installieren hinzu.<br>
	 * Diese Methode startet im Hintergrund das Auslesen von Informationen aus dem Package<br>
	 * Diese Methode darf nicht mehr ausgeführt werrden, wenn die Installation bereits begonnen wurde!
	 * @param fileName Pfad zum Package
	 * @return liefert false falls schon jetzt erkannt wird, dass dieses Package nicht installiert werden kann (zB schon in der Liste vorhanden)
	 */
	bool AddPackageToInstall(const std::wstring& fileName);

	/**
	 * Entfernt ein Package aus der Liste zu installierender Packages<br>
	 * Diese Methode darf nicht mehr ausgeführt werrden, wenn die Installation bereits begonnen wurde!
	 * @param fileName Pfad zum Package
	 * @return liefert true falls package entfernt wurde; false falls es nicth in der liste vorhanden ist
	 */
	bool RemovePackageToInstall(const std::wstring& fileName);
	
	bool IsPreInstallFinished(const std::wstring& packageFileName);

	const std::wstring& GetReadmeForPackage(const std::wstring& packageFileName);

	void CancelAndWaitForTasks();

	void CancelInstallation();

	/**
	 * Startet Installation der Packages im Hintergrund
	 */
	void StartInstallation();

	/**
	 * Startet zweiten Durchgang der Installation<br>
	 * Installiert / Löscht alle Dateien die im übergebenen Argument gesetzt sind<br>
	 * Setzt das EventInstallFinished zurück. Dieses Event kann nun wieder verwendet werden, um auf den Abschluss der Installation zu warten
	 * Aufruf nur gültig falls IsAlwaysAsk() true liefert
	 * @param userSelection Auswahl des Benutzers welche Dateien in welchem Package zu installieren sind. Dateien müssen sortiert sein!!! (Std operator< auf wstrings)
	 */
	void StartInstallUserSelection(std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> userSelection);

	/**
	 * Liefert Informationen zum aktuellen Informationsvortschritt
	 * @param cntProcessedFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der bereits verarbeiteten Dateien hineingeschrieben
	 * @param cntTotalFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der insgesamt zu verarbeiteten Dateien hineingeschrieben
	 * @return Fortschritt in Prozent (ungefährer Wert, nicht darauf verlassen, dass dieser am Ende 100 ist!)
	 */
	size_t GetInstallationProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles);

	inline void SetReplaceOnlyOlder(bool _replaceOnlyOlder) { replaceOnlyOlder = _replaceOnlyOlder; }
	inline void SetAlwaysAskBeforeOverwrite(bool _alwaysAskBeforeOverwrite) { alwaysAskBeforeOverwrite = _alwaysAskBeforeOverwrite; }

	inline const Concurrency::event& GetEventInstallFinished() { return eventInstallFinished; }

	inline bool IsInstallFinished() { return eventInstallFinished.wait(0) == 0; }

	inline InstallManagerState GetInstallerState() const { return installState; }

	inline const InstallerException* GetInstallerException() const { return errorEx.get(); }

	inline bool IsUsingTransaction() const { return usedTransactions; }

	inline bool IsAlwaysAsk() const { return alwaysAskBeforeOverwrite; }

	inline const std::map<std::wstring, std::unique_ptr<insthelper::InstallInformation>>* GetInstallInformation() const { return &installInformation; }

	inline std::vector<std::wstring>& GetSelectedFilesToInstall() { return selectedFilesToInstall; }

	inline const boost::filesystem::path& GetL3dPath() const { return l3dPath; }

	inline std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> GetUserSelection() const { return userSelectedFiles; }

	// Liste von Packages mit einem Pair<PackageName, Name deinstallierter Dateien>
	std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> GetDeinstalledFiles() const;

	// Liste von Packages mit einem Pair<PackageName, Name zu deinstallierender Dateien>
	std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> GetFilesToDeinstall() const;

	//inline std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> GetFilesToDeinstall() const { return filesToDeinstall; }


protected:
	void InstallPackage(size_t numThreads, const RootPackageInfo& rootPkg, const PackageInfo& pkgInfo, insthelper::InstallInformation *instInfo);
	void BackupFile(const boost::filesystem::path& l3dRelativeFilePath, insthelper::InstallInformation* installInfo);
	
	template<class Func>
	void DoExceptionHandledInstall(Func f);

private:
	void InstallFiles(size_t startIndex, size_t end, const RootPackageInfo& rootPkg, const PackageInfo& pkgInfo, insthelper::InstallInformation *installInfo);
	void DeleteFiles(const std::vector<std::wstring>& filesToDelete, insthelper::InstallInformation* instInfo, const PackageInfo& pkgInfo);
	void ThrowZipException(ZRESULT zipRes, const PackageInfo& pkgInfo, const char *exMsg, const std::wstring& curFile);
	void SetBackupDir(RootPackageInfo& pkgInfo);
	bool IsInDeleteList(const std::wstring& fname, const insthelper::InstallInformation& installInfo);
	std::vector<int> DetermineFilesToDeinstall();
	bool IsInDeinstallList(const std::wstring& fname);

	// Erzeugt Liste zu deinstallierender Dateien zur Anzeige für User
	void GenerateFilesToDeinstallList(std::vector<int> deinstallPkgIds);
	// Prüft ob eine Datei in installInformation drinnen ist
	bool IsInInstallList(const std::wstring& fname) const;

	std::vector<std::wstring> selectedFilesToInstall;
	std::map<std::wstring, std::unique_ptr<PreInstallWorker>> installData;
	std::map<std::wstring, std::unique_ptr<insthelper::InstallInformation>> installInformation;
	std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> userSelectedFiles;

	Concurrency::task_group bgWork;
	Concurrency::task_group workerTskGrp;
	Concurrency::event eventInstallFinished;

	KTMTransaction transactFs;

	std::unique_ptr<InstallerException> errorEx;
	
	bool replaceOnlyOlder;
	bool alwaysAskBeforeOverwrite;
	boost::filesystem::path l3dPath;
	boost::filesystem::path rootBackupPath;
	boost::filesystem::path currentBackupPath;

	size_t cntTotalFiles;

	InstallManagerState installState;
	bool usedTransactions;
	boost::atomic<bool> cancelled;

	db::InstallPkgDBAgent dbAgent;

	Concurrency::reader_writer_lock installInfoLock;

	std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> filesToDeinstall;

	std::shared_ptr<std::map<int, std::vector<std::wstring>>> deinstalledFiles;

	// Mapping ID -> Name für Packages die zu deinstallieren sind
	std::shared_ptr<std::map<int,  db::DBPackageEntry>> packagesToDeinstall;

	// Liste zu deinstallierender Dateien zur Anzeige für User
	// vector<pair<PackageName, Liste zu deinstalierender Dateien>>
	std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> userFilesToDeinstall;

private:
	InstallManager(const InstallManager&);
	InstallManager& operator=(const InstallManager&);
};

}
}