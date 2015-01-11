#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <vector>
#include <array>
#include <string>

#include "FileDlg.h"

/**
 * Interface f�r s�mtliche Datei Dialoge zum Speichern von Dateien
 */
class SaveFileDlg : public FileDlg
{
public:
	virtual ~SaveFileDlg() {}

	/**
	* Liefert die ausgew�hlte Datei, falls User welche gew�hlt hat<br>
	* Falls nichts ausgew�hlt, empty string
	* @return bsoluter Datei-Pfad
	*/
	virtual std::wstring GetResult() = 0;
};


