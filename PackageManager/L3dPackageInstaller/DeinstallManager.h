#pragma once

#include <vector>
#include <string>
#include <utility>
#include <memory>
#include <map>
#include <exception>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include <ppl.h>
#include <agents.h>

#include <lhWinAPI.h>

#include <KompexSQLiteDatabase.h>
#include "KTM.h"
#include "AgentCommand.h"

#include "FileToDeinstall.h"

namespace l3d
{
namespace packageinstaller
{

/**
 * Exception die bei Uninstall von Packages die Fehlermeldungen weitergibt
 */
class DeinstallException : public std::exception
{
public:
	DeinstallException(int pkgId, const std::wstring& filename, const std::wstring& msg) : std::exception("Deinstall Exception"), pkgId(pkgId), msg(msg), filename(filename)
	{
	}

public:
	DeinstallException(int pkgId, const std::wstring& filename, const std::string& msg) : std::exception(msg.c_str()), pkgId(pkgId), filename(filename)
	{
		lhWinAPI::CharToTchar(this->msg, msg);
	}

	/**
	 * ID des Package in DB welches gerade bearbeitet wurde
	 */
	int GetPackageId() const { return pkgId; }
	/**
	 * Fehlermeldung
	 */
	const std::wstring& GetMsg()  const { return msg; }
	/**
	 * Dateiname die gerade zu deinstallieren versucht wurde (kann auch leer sein, falls es ein allgemeiner (DB-)Fehler war)
	 */
	const std::wstring& GetProcessedFilename() const { return filename; }

private:
	int pkgId;
	std::wstring msg;
	std::wstring filename;
};


/*------------------------------------------------------------------------*/

/**
 * Basisklasse f�r alle Uninstall-Agents
 */
class BaseDeinstallAgent : boost::noncopyable, public Concurrency::agent
{
public:
	virtual ~BaseDeinstallAgent() { }

	/**
	 * Sendet einen Befehl an den Agent
	 */
	virtual void SetAgentCommand(AgentCommand cmd) { send(cmdBuffer, cmd); }

protected:
	/**
	 * Letzter g�ltiger Befehl der mit SetAgentCommand gesetzt wurde
	 */
	Concurrency::overwrite_buffer<AgentCommand> cmdBuffer;
};

/*------------------------------------------------------------------------*/

/**
 * Basisklasse f�r alle Agents die ThrowExceptionIfAvailable() unterst�tzen
 */
class ThrowableDeinstallAgent : public BaseDeinstallAgent
{
public:
	/**
	 * Wirt eine Exception falls der Agent im Hintergrund-Task einen Fehler geworfen hat
	 */
	void ThrowExceptionIfAvailable();

protected:
	/**
	 * Error buffer der mit DeinstallException gef�llt werden kann<br>
	 * Falls etwas in diesem Buffer steht, wirft ThrowExceptionIfAvailable eine Exception beim Aufruf
	 */
	Concurrency::single_assignment<DeinstallException> errBuffer;
};

/*------------------------------------------------------------------------*/

/**
 * Class that allows agent to report progress
 */
class ProgressReportableAgent : public ThrowableDeinstallAgent
{
public:
	ProgressReportableAgent(void);
	/**
	 * Liefert Informationen zum aktuellen Fortschritt beim Bestimmen der zu deinstallierenden Dateien
	 * @param cntProcessedFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der bereits verarbeiteten Dateien hineingeschrieben
	 * @param cntTotalFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der insgesamt zu verarbeiteten Dateien hineingeschrieben
	 * @return Fortschritt in Prozent (ungef�hrer Wert, nicht darauf verlassen, dass dieser am Ende 100 ist!)
	 */
	virtual size_t GetProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles);

protected:
	size_t cntTotalFiles;
	Concurrency::overwrite_buffer<size_t> cntProcessedFilesBuffer;
};

/*------------------------------------------------------------------------*/

class RemoveFilesAgent;

/**
 * Agent der die eigentliche Deinstallation der Dateien + L�schen aus der DB vornimmt
 */
class DeinstallAgent : public ProgressReportableAgent
{
public:
	/**
	 * Instanziert einen DeinstallAgent der s�mtliche nicht mehr verwendeten Dateien der zu l�schenden Packages von der DB und Disk l�scht
	 * @param l3dPath Pfad zum Loksim-Verzeichnis
	 * @param pkgIdsToDelete IDs der Packages in DB die deinstalliert werden sollen
	 */
	DeinstallAgent(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete);

	/**
	 * Instanziert einen DeinstallAgent der s�mtliche nicht mehr verwendeten Dateien der zu l�schenden Packages von der DB und Disk l�scht
	 * @param l3dPath Pfad zum Loksim-Verzeichnis
	 * @param pkgIdsToDelete IDs der Packages in DB die deinstalliert werden sollen
	 * @param dbConnection Connection zur DB die verwendet werden soll (macht kein Rollback oder Commit!)
	 * @param transactFs Filesystem Transaktion die verwendet werden soll (macht kein Rollback oder Commit!)
	 */
	DeinstallAgent(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete, Kompex::SQLiteDatabase* dbConnection, KTMTransaction* transactFs);

	/**
	 * Instanziert einen DeinstallAgent der alle �bergebenen Dateien von der Disk l�scht, und S�MTLICHE nicht mehr verwendeten Dateien der Packages aus der DB
	 * @param l3dPath Pfad zum Loksim-Verzeichnis
	 * @param packagesAndFilesToDelete	Keys: IDs der Packages die deinstalliert werden sollen
	 *									Values: Vektor mit Dateinamen (relativ zum L3D-Verzeichnis) die f�r jedes Package tats�chlich von der Disk gel�scht werden sollen
	 */
	DeinstallAgent(const boost::filesystem::path& l3dPath,  std::shared_ptr<std::map<int, std::vector<std::wstring>>> packagesAndFilesToDelete);

	~DeinstallAgent(void);

	/**
	 * Liefert Informationen zum aktuellen Fortschritt beim Bestimmen der zu deinstallierenden Dateien
	 * @param cntProcessedFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der bereits verarbeiteten Dateien hineingeschrieben
	 * @param cntTotalFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der insgesamt zu verarbeiteten Dateien hineingeschrieben
	 * @return Fortschritt in Prozent (ungef�hrer Wert, nicht darauf verlassen, dass dieser am Ende 100 ist!)
	 */
	virtual size_t GetProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles);
	
	/**
	 * Sendet einen Befehl an den Agent
	 */
	void SetAgentCommand(AgentCommand cmd);

	/**
	 * Holt die gel�schten Dateien ab<br>
	 * Aufruf nur g�ltig, falls Agent fertig ist<br>
	 * @returns Map mit gel�schten Dateien. Key = PackageID, Value = Vektor mit Dateien die f�r dieses package gel�scht wurden
	 */
	std::shared_ptr<std::map<int, std::vector<std::wstring>>> GetDeinstalledFiles() { return deletedFiles; }

	/**
	 * Bestimmt ob der Agent Transaktionen benutzt und deshalb ein Abbruch problemlos m�glich ist
	 * @return true falls Transaktionen eingesetzt werden
	 */
	bool IsUsingTransactions() const;

	void SetUndoInstallation(bool undo) { this->undoInstallation = undo; }

protected:
	virtual void run();

private:
	/**
	 * Liefert zur�ck ob es "eigene Transaktion" ist oder Transaktion von au�en gesteuert wird
	 */
	bool IsOwnTransaction() const;

	boost::filesystem::path l3dPath;
	boost::filesystem::path rootBackupPath;
	std::vector<int> pkgIdsToDelete;
	std::unique_ptr<Kompex::SQLiteDatabase> updb;
	std::shared_ptr<RemoveFilesAgent> curAgent;
	std::shared_ptr<std::map<int, std::vector<std::wstring>>> packagesAndFilesToDelete;
	std::shared_ptr<std::map<int, std::vector<std::wstring>>> deletedFiles;
	size_t cntProcessedBefore;
	bool usingTxFs;
	bool undoInstallation;
	Kompex::SQLiteDatabase* db;
	KTMTransaction* transactFs;
};

/*------------------------------------------------------------------------*/

/**
 * Agent der s�mtliche Dateien 'sammelt' die beim Deinstallieren von Packages gel�scht werden sollten
 */
class FilesToDeinstallGathererAgent : public ProgressReportableAgent
{
public:
	/**
	 * Instanziert neuen Agent
	 * @param l3dPath Pfad zum Loksim-Verzeichnis
	 * @param pkgIdsToDelete IDs der Packages in DB f�r welche die zu deinstallierenden Dateien gesammelt werden sollen
	 */
	FilesToDeinstallGathererAgent(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete);

	/**
	 * Holt die zu l�schenden Dateien ab<br>
	 * Aufruf nur g�ltig, falls Agent fertig ist<br>
	 * @returns Map mit zu l�schenden Dateien. Key = PackageID, Value = Vektor mit Dateien die f�r dieses package gel�scht werden sollten
	 */
	std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> GetFilesToDeinstall() { return filesToDeinstall; }

protected:
	virtual void run();

private:
	boost::filesystem::path l3dPath;
	std::vector<int> pkgIdsToDelete;
	std::unique_ptr<Kompex::SQLiteDatabase> db;
	std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> filesToDeinstall;
};

/*------------------------------------------------------------------------*/

/**
 * Agent der das L�schen der Dateien im Dateisystem vornimmt
 */
class RemoveFilesAgent : public BaseDeinstallAgent
{
public:
	RemoveFilesAgent(const boost::filesystem::path& l3dPath, bool logDeletedFiles, KTMTransaction* transactFs);

	bool HasError() const { return errBuffer.has_value(); }
	/**
	 * Liefert ein Pair mit folgendem Inhalt:<br>
	 * first: Dateiname der gerade bearbeitet wurde als Fehler aufgetreten ist<br>
	 * second: Fehlermeldung<br>
	 * Aufruf nur g�ltig, falls HasError() true liefert! Ansonsten blockiert diese Funktion!
	 */
	const std::pair<std::wstring, std::wstring>& GetError() { return errBuffer.value(); }

	/**
	 * Liefert Puffer in welchen die zu l�schenden Dateien geschrieben werden k�nnen. Dateinamen relativ zum L3D-Verzeichnis!<br>
	 * Falls ein Leer-String geschrieben wird, wartet der Agent auf den abschlie�enden Command
	 */
	Concurrency::ITarget<std::wstring>* GetFilesToDeleteTarget() { return &filesToDeleteBuffer; }

	/**
	 * Falls logDeletedFiles true �bergeben wurde, liefert diese Methode die gel�schten Dateien
	 * Methode kann nur einmal aufgerufen werden!
	 */
	std::shared_ptr<std::vector<std::wstring>> GetDeletedFiles() { return deletedFiles; }

	/**
	 * Liefert anzahl verarbeiteter Dateien thread-safe zur�ck
	 */
	size_t GetCntProcessedFiles();

	/**
	 * Bestimmt ob der Agent Transaktionen benutzt und deshalb ein Abbruch problemlos m�glich ist
	 * @return true falls Transaktionen eingesetzt werden
	 */
	bool IsUsingTxFs() const;

	void SetBackupDir(const boost::filesystem::path& backupDir) { this->backupDir = backupDir; }

	void SetUndoInstallation(bool undoInstallation) { this->undoInstallation = undoInstallation; }

	/**
	 * Liefert zur�ck ob es eine eigene Transaktion ist oder ob Transaktion von au�en gesteuert wird
	 */
	bool IsOwnTransaction() const;

protected:
	virtual void run();

private:
	Concurrency::unbounded_buffer<std::wstring> filesToDeleteBuffer;
	Concurrency::single_assignment<std::pair<std::wstring, std::wstring>> errBuffer;

	boost::filesystem::path l3dPath;
	std::unique_ptr<KTMTransaction> ownTxFs;
	KTMTransaction* txFs;

	bool usingTxFs;

	bool logDeletedFiles;
	std::shared_ptr<std::vector<std::wstring>> deletedFiles;
	Concurrency::overwrite_buffer<size_t> cntProcessedFilesBuffer;

	boost::filesystem::path backupDir;
	bool undoInstallation;
};

/*------------------------------------------------------------------------*/

class DeinstallManager : boost::noncopyable
{
public:
	/**
	 * Definiert die m�glichen Zust�nde des Managers
	 */
	enum DeinstallState
	{
		DeinstallNotRunning, DeinstallGatheringFiles, DeinstallRunning, DeinstallGatheringFinished, DeinstallFinished, DeinstallError, DeinstallCancelled
	};

public:
	DeinstallManager(const boost::filesystem::path& l3dPath,  const std::vector<int>& pkgIdsToDelete);
	~DeinstallManager(void);

	/**
	 * Startet im Hintergrund das Sammeln der zu l�schenden Dateien f�r die �bergebenen Packages
	 */
	void StartGetFilesToDelete();
	/**
	 * Startet im Hintergrund die Deinstallation der Packages
	 */
	void StartDeinstall();	
	/**
	 * Startet im Hintergrund die Deinstallation der Packages anhand einer benutzerdefinierten Datei-Auswahl
	 * @param packagesAndFilesToDelete	Keys: IDs der Packages die deinstalliert werden sollen
	 *									Values: Vektor mit Dateinamen (relativ zum L3D-Verzeichnis) die f�r jedes Package tats�chlich von der Disk gel�scht werden sollen
	 */
	void StartDeinstall(std::shared_ptr<std::map<int, std::vector<std::wstring>>> packagesAndFilesToDelete);
	/**
	 * Liefert zur�ck ob der aktuelle Hintergrund Task abgeschlossen ist
	 */
	bool IsFinished();
	/**
	 * Wartet bis der aktuelle Task beendet wurde<br>
	 * If the parameter _Timeout has a value other than the constant COOPERATIVE_TIMEOUT_INFINITE,
	 * the exception operation_timed_out is thrown if the specified amount of time expires before the agent has completed its task.
	 * @param _Timeout The maximum time for which to wait, in milliseconds. 
	 */
	void Wait(unsigned int _Timeout = Concurrency::COOPERATIVE_TIMEOUT_INFINITE);

	/**
	 * Liefert Informationen zum aktuellen Fortschritt beim aktuellen Deinstallations-Schritt
	 * @param cntProcessedFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der bereits verarbeiteten Dateien hineingeschrieben
	 * @param cntTotalFiles [optional] Wenn nicht nullptr, wird hier die Anzahl der insgesamt zu verarbeiteten Dateien hineingeschrieben
	 * @return Fortschritt in Prozent (ungef�hrer Wert, nicht darauf verlassen, dass dieser am Ende 100 ist!)
	 */
	size_t GetProgress(size_t *cntProcessedFiles, size_t *cntTotalFiles);

	/**
	 * Liefert den Status der zuletzt gestarteteten Deinstallations-Operation
	 */
	DeinstallState GetState();

	/**
	 * Bricht aktuelle Deinstallations-Operation ab
	 */
	void CancelDeinstall();

	/**
	 * Bricht alle laufenden Operationen ab, und wartet bis s�mtliche BG-Tasks beendet sind
	 */
	void CancelAndWaitForTasks();

	/**
	 * Liefert Deinstall Exception oder nullptr falls keine Exception aufgetreten ist
	 */
	DeinstallException* GetDeinstallError() { return deinstallException.get(); }

	/**
	 * Holt die gel�schten Dateien ab<br>
	 * Aufruf nur g�ltig, falls IsFinished true und zuvor StartDeinstall() (ohne Argumente) aufgerufen wurde<br>
	 * Diese Methode kann nur einmal aufgerufen werden!
	 * @returns Map mit gel�schten Dateien. Key = PackageID, Value = Vektor mit Dateien die f�r dieses package gel�scht wurden
	 */
	std::shared_ptr<std::map<int, std::vector<std::wstring>>> GetDeinstalledFiles();

	/**
	 * Holt die zu l�schenden Dateien ab<br>
	 * Aufruf nur g�ltig, falls IsFinished true und zuvor StartGetFilesToDelete() aufgerufen wurde<br>
	 * Diese Methode kann nur einmal aufgerufen werden!
	 * @returns Map mit zu l�schenden Dateien. Key = PackageID, Value = Vektor mit Dateien die f�r dieses package gel�scht werden sollten
	 */
	std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> GetFilesToDeinstall();

	/**
	 * Bestimmt ob der Agent Transaktionen benutzt und deshalb ein Abbruch problemlos m�glich ist
	 * @return true falls Transaktionen eingesetzt werden
	 */
	bool IsUsingTransactions() const;

	/**
	 * Falls undo auf true gesetzt wird, wird eine Installation r�ckg�ngig gemacht<br>
	 * d.h. es wird kein Backup der deinstallierten Files erstellt, sondern das Backup von der Installation zur�ckgespielt
	 */
	void SetUndoInstallation(bool undo) { this->undoInstallation = undo; }

private:
	boost::filesystem::path l3dPath;
	std::vector<int> packagesToDeinstall;
	std::shared_ptr<ProgressReportableAgent> agent;
	DeinstallState deinstallState;
	std::unique_ptr<DeinstallException> deinstallException;
	bool undoInstallation;
};

}
}