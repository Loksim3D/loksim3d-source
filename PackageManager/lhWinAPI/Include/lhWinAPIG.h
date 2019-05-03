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
	// Class MenuDis �ernimmt enablen und grayen von mehreren Menupunkten 	
	class MenuDis
	{
	public:
		//*********************************************
		//* Erstellt eine Class MenuDis f�rs Men� menu 
		//*********************************************
		MenuDis(HMENU menu = 0);
		//***************************************
		// Grayed alle gesetzten Men�punkte aus 
		//***************************************
		void Disable() const;
		//***********************************
		// enabled alle gesetzten Men�punkte 
		//***********************************
		void Enable() const;
		//*******************************
		// Setzt das zu verwaltende Men� 
		//*******************************
		void SetMenu(HMENU hMenu);
		//********************************************
		// F�gt einen zu verwaltenden Men�punkt hinzu 
		//********************************************
		void AddPoint(int point);
		//************************
		// L�scht alle Men�punkte 
		//************************
		void Clear();
	private:
		HMENU hMenu;
#pragma warning( disable : 4251 )
		std::vector<int> vec;
#pragma warning( default : 4251 )
	};
}