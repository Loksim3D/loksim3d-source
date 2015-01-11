#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <vector>
#include <array>
#include <string>

#include "FileDlg.h"


/**
 * Interface f�r s�mtliche Datei Dialoge zum �ffnen von Dateien
 */
class OpenFileDlg : public FileDlg
{
public:
	virtual ~OpenFileDlg() { }

	/**
	* Legt fest ob mehrere Dateien auf einmal ausgew�hlt werden k�nnen
	* @param bAllow legt fest ob MultiSelect erlaubt ist
	* @return S_OK im Erfolgsfall
	*/
	virtual HRESULT SetAllowMultiSelect(bool bAllow) = 0;

	/**
	* Liefert die ausgew�hlten Dateien, falls User welche gew�hlt hat<br>
	* Falls MultiSelect nicht erlaubt war, ist nur ein Element im Vector
	* @return Vector von absoluten Datei-Pfaden
	*/
	virtual std::vector<std::wstring> GetResults() = 0;
};


