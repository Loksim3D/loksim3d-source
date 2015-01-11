#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <vector>
#include <array>
#include <string>

#include "FileDlg.h"

/**
 * Interface für sämtliche Datei Dialoge zum Speichern von Dateien
 */
class SaveFileDlg : public FileDlg
{
public:
	virtual ~SaveFileDlg() {}

	/**
	* Liefert die ausgewählte Datei, falls User welche gewählt hat<br>
	* Falls nichts ausgewählt, empty string
	* @return bsoluter Datei-Pfad
	*/
	virtual std::wstring GetResult() = 0;
};


