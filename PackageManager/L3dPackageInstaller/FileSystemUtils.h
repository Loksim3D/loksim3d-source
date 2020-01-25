#pragma once

#include <boost/filesystem.hpp>
#include <string>
#include "KTM.h"

namespace l3d
{
namespace packageinstaller
{
namespace fs
{
	/**
	 * Legt ein Verzeichnis an (sämtliche Verzeichnisse im Pfad die nicht existieren werden angelegt) und verwendet die NTFS-Transaktion
	 */
	bool CreateDirectoriesTransacted(KTMTransaction& trans, const boost::filesystem::path& dir);
	/**
	 * Prüft mit der übergebenen Transaktion ob eine Datei existiert
	 */
	bool FileExistsTransacted(KTMTransaction& trans, const wchar_t *fileName);
	/**
	 * Verschiebt eine ganze Verzeichnisstruktur + Unterverzeichnisse in einer Transaktion
	 * @param cancel Falls dieser Pointer != nullptr und während der Abarbeitung auf true gesetzt wird, wird Verschieben abgebrochen
	 */
	void MoveDirectory(KTMTransaction& trans, const wchar_t* fromDir, const wchar_t* toDir, bool *cancel = nullptr);

	boost::filesystem::path LoksimRelativeToAbsolutePath(const std::wstring& relativePath, const boost::filesystem::path& l3dDir);

	std::wstring AbsoluteToLoksimRelativePath(const boost::filesystem::path& absolutePath, const boost::filesystem::path& l3dDir);

	// Liefert vollen Pfad für Datei in Local-Settings Ordner des Loksims (LOCAL_APPDATA/Loksim3D)
	// Erstellt Verzeichnis falls es nicht existiert
	std::wstring GetFilenameInLocalSettingsDir(const std::wstring& file);

	const static std::wstring FILENAME_CRASHRPT_LOG = L"LetztePkgManagerInstallation.txt";
}
}
}