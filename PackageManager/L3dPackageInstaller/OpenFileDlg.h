#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <vector>
#include <array>
#include <string>

#include "FileDlg.h"


/**
 * Interface für sämtliche Datei Dialoge zum Öffnen von Dateien
 */
class OpenFileDlg : public FileDlg
{
public:
	virtual ~OpenFileDlg() { }

	/**
	* Legt fest ob mehrere Dateien auf einmal ausgewählt werden können
	* @param bAllow legt fest ob MultiSelect erlaubt ist
	* @return S_OK im Erfolgsfall
	*/
	virtual HRESULT SetAllowMultiSelect(bool bAllow) = 0;

	/**
	* Liefert die ausgewählten Dateien, falls User welche gewählt hat<br>
	* Falls MultiSelect nicht erlaubt war, ist nur ein Element im Vector
	* @return Vector von absoluten Datei-Pfaden
	*/
	virtual std::vector<std::wstring> GetResults() = 0;
};


