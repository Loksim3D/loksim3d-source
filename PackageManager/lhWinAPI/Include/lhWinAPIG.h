#pragma once
#include <windows.h>

#include <string>
#include <vector>
#include <string>

namespace lhWinAPI
{
	//*********************************************************************************
	// Mapped char String zu wchar_t string (gleiche Parameter wie MultiByteToWideChar() 
	int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, const std::basic_string<char>& sAsciiStr, std::basic_string<wchar_t>& sWideCharStr);
	//*********************************************************************************
	// Mapped char String zu wchar_t string (gleiche Parameter wie MultiByteToWideChar() 
	int MultiByteToWideChar(UINT CodePage, DWORD dwFlags, const char *szAsciiStr, int cbMultiByte, std::basic_string<wchar_t>& sWideCharStr);
	//*********************************************************************************
	// Mapped wchar_t String zu char string (gleiche Parameter wie WideCharToMultiByte() 
	int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, const std::wstring& sWideCharStr, std::string& sAsciiStr, 
								   const char *psDefaultChar, LPBOOL lpUsedDefaultChar);
	//*********************************************************************************
	// Mapped wchar_t String zu char string (gleiche Parameter wie WideCharToMultiByte() 
	int WideCharToMultiByte(UINT CodePage, DWORD dwFlags, const wchar_t *sWideCharStr, int cchWideChar, std::string& sAsciiStr, 
								   const char *psDefaultChar, LPBOOL lpUsedDefaultChar);

	//**********************************************************************
	// Class MenuDis üernimmt enablen und grayen von mehreren Menupunkten 	
	class MenuDis
	{
	public:
		//*********************************************
		//* Erstellt eine Class MenuDis fürs Menü menu 
		//*********************************************
		MenuDis(HMENU menu = 0);
		//***************************************
		// Grayed alle gesetzten Menüpunkte aus 
		//***************************************
		void Disable() const;
		//***********************************
		// enabled alle gesetzten Menüpunkte 
		//***********************************
		void Enable() const;
		//*******************************
		// Setzt das zu verwaltende Menü 
		//*******************************
		void SetMenu(HMENU hMenu);
		//********************************************
		// Fügt einen zu verwaltenden Menüpunkt hinzu 
		//********************************************
		void AddPoint(int point);
		//************************
		// Löscht alle Menüpunkte 
		//************************
		void Clear();
	private:
		HMENU hMenu;
#pragma warning( disable : 4251 )
		std::vector<int> vec;
#pragma warning( default : 4251 )
	};
}