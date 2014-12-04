#pragma once

#include <string>
#include <ctime>

namespace l3d
{
namespace packageinstaller
{

/**
 * Kapselt Informationen über eine zu deinstallierende Datei<br>
 * Diese Infos werden dem Benutzer angezeigt, falls er eine explizitie Auswahl an Dateien vornehmen möchte
 */
struct FileToDeinstall
{
	FileToDeinstall(const std::wstring& filename, bool writeProtected, const std::time_t& modifiedTimestamp) :
		filename(filename), writeProtected(writeProtected), modifiedTimestamp(modifiedTimestamp)
	{
	}
	/**
	 * Name der Datei (relativ zu L3D-Verzeichnis)
	 */
	std::wstring filename;
	/**
	 * Ist Datei schreibgeschützt?
	 */
	bool writeProtected;
	/**
	 * Modified TimeStamp der Datei auf der Platte
	 */
	std::time_t modifiedTimestamp;
};

}
}