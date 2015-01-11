#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <vector>
#include <array>
#include <string>

/**
 * Interface f�r s�mtliche Datei Dialoge zum �ffnen bzw. Speichern von Dateien
 */
class FileDlg
{
public:
	virtual ~FileDlg() {}

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
	 * Legt default extension feste
	 * @param extension Default extension ohne '.' zb 'jpg'
	 * @return S_OK im Erfolgsfall
	 */
	virtual HRESULT SetDefaultExtension(const std::wstring& extension) = 0;

	virtual HRESULT SetClientGuid(const GUID& guid) { return E_NOTIMPL; }
};


