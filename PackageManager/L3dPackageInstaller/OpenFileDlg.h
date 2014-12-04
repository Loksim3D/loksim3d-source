#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <vector>
#include <array>
#include <string>

/**
 * Interface f�r s�mtliche Datei Dialoge zum �ffnen von Dateien
 */
class OpenFileDlg
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
	 * Legt fest ob ausgew�hlte Datei (und damit auch Pfad) existieren muss
	 * @param bMustExist legt fest ob datei existieren muss
	 * @return S_OK im Erfolgsfall
	 */
	virtual HRESULT SetPathAndFileMustExist(bool bMustExist) = 0;
	/**
	 * Gibt die FileTypes (den Filter) an, die der User ausw�hlen kann
	 * @param filterSpec Vector von Filterspezifikationen
	 * @return S_OK im Erfolgsfall
	 */
	virtual HRESULT SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& filterSpec) = 0;
	/**
	 * Legt den Titel des Dialogs fest
	 * @param title Titel
	 * @return S_OK im Erfolgsfall
	 */
	virtual HRESULT SetTitle(const std::wstring& title) = 0;
	/**
	 * Zeigt den Dialog modal an
	 * @param hwndOwner Fenster des Aufrufers
	 * @return S_OK im Erfolgsfall
	 */
	virtual HRESULT Show(HWND hwndOwner) = 0;
	/**
	 * Liefert die ausgew�hlten Dateien, falls User welche gew�hlt hat<br>
	 * Falls MultiSelect nicht erlaubt war, ist nur ein Element im Vector
	 * @return Vector von absoluten Datei-Pfaden
	 */
	virtual std::vector<std::wstring> GetResult() = 0;
};


