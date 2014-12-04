#pragma once
#include <windows.h>
#include <commctrl.h>
#include <sstream>
namespace lhWinAPI
{
	//******************************************
	//* Setzt den WindowText auf wert vom Typ T
	//******************************************
	template<class T> void SetWindowValue(const HWND hwnd, const T wert);

	//***********************************************
	//* Liest Wert vom Typ T aus WindowText hwnd aus     
	//***********************************************
	template<class T> T GetWindowValue(HWND hwnd, T wert);

	//***************************************
	//* Liest string aus WindowText hwnd aus 
	//***************************************
	 std::wstring GetWindowString(HWND hwnd);

	//****************************************
	//* Fügt Element zu Treeview hwndTV hinzu
	//****************************************
	 HTREEITEM AddItemToTree(HWND hwndTV, std::basic_string<wchar_t> nameItem, int nLevel, LPARAM lParam, int image);

	//*********************************************
	//* Suche nach Ordner (BrowseForFolder Dialog)
	//*********************************************
	 std::wstring BrowseForFolder(HWND hwnd, std::basic_string<wchar_t> title);

	//****************************************************
	//* Liefert den letzten Fehler mittels GetLastError() 
	std::wstring GetLastErrorS();

	//****************************************************
	//* Liefert eine Fehlermeldung auf Basis von dwError 
	std::wstring GetLastErrorS(DWORD dwError);

	//*************************************************************
	//* Liefert den absoluten Pfad von fileName im EXE Verzeichnis 
	//*************************************************************
	std::basic_string<wchar_t> FilePathExec(std::basic_string<wchar_t> fileName);

	//********************************************
	//* Testet ob die Datei fileName existiert
	bool FileExists(const std::wstring& fileName);

	//**************************************************************************************************************
	//* Speichert die Version der Datei sFileName auf dest, isPreRelease wird true falls die Version PreRelease ist 
	// liefert true falls keine Fehler aufgetreten sind
	bool GetFileVersion(std::wstring& sFileName, std::wstring& dest, bool* isPreRelease);

	//**************************************************************************************************
	// Konvertiert den TCHAR-String src in den char-String dest; Liefert Anzahl umgewandelter Zeichen zurück
	int TcharToChar(std::string& dest, const std::wstring& src);
	//**************************************************************************************************
	// Konvertiert den Char-String src in den TCHAR-String dest; Liefert Anzahl umgewandelter Zeichen zurück	
	int CharToTchar(std::wstring& dest, const std::string& src);
	//**************************************************************************************************
	// Konvertiert den Wchar-String src in den TCHAR-String dest; Liefert Anzahl umgewandelter Zeichen zurück	
	int WcharToTchar(std::wstring& dest, const std::wstring& src);
	//**************************************************************************************************
	// Konvertiert den Tchar-String src in den Wchar-String dest; Liefert Anzahl umgewandelter Zeichen zurück	
	int TcharToWchar(std::wstring& dest, const std::wstring& src);

	//****************************************************************************
	// Stellt einfach Funktionen fürs Laden von Strings auf Resource zur Verfügung
	class StringTable
	{
	public:
		//************************************************
		//* Liefert einen std::basic_string<> auf Message 
		//************************************************
		std::basic_string<wchar_t> LoadS(UINT msg);
		//***********************************
		//* Liefert einen TCHAR* auf Message 	
		//***********************************
		wchar_t* Load(UINT msg);

	private:
		wchar_t buffer[1024];
	};
}

//******************************************
//* Setzt den WindowText auf wert vom Typ T
//******************************************
template<class T> void lhWinAPI::SetWindowValue(const HWND hwnd, const T wert)
{
	std::basic_ostringstream<wchar_t> os;

	os << wert;
	SetWindowText(hwnd, (os.str()).c_str());
}


//***********************************************
//* Liest Wert vom Typ T aus WindowText hwnd aus     
//***********************************************
template<class T> T lhWinAPI::GetWindowValue(HWND hwnd, T wert)
{
	std::basic_istringstream<wchar_t> is;
	wchar_t szBuffer[1024] = { 0 };

	GetWindowText(hwnd, szBuffer, 1023);
	std::basic_string<wchar_t> s(szBuffer);
	is.str(s);
	is >> wert;

	return wert;
}
