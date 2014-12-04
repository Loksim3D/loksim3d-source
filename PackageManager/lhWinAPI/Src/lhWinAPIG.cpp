#include "..\Include\lhWinAPIG.h"
#include <lhException.h>

#include <memory>

//*********************************************
//* Erstellt eine Class MenuDis f�rs Men� menu 
//*********************************************
lhWinAPI::MenuDis::MenuDis(HMENU menu) : hMenu(menu), vec(0)
{
}

//****************************************
//* Grayed alle gesetzten Men�punkte aus *
//****************************************
void lhWinAPI::MenuDis::Disable() const
{
	for (std::vector<int>::const_iterator iter = vec.begin(); iter != vec.end(); iter++)
		EnableMenuItem(hMenu, *iter, MF_GRAYED);
}

//*************************************
//* enabled alle gesetzten Men�punkte *
//*************************************
void lhWinAPI::MenuDis::Enable() const
{
	for (std::vector<int>::const_iterator iter = vec.begin(); iter != vec.end(); iter++)
		EnableMenuItem(hMenu, *iter, MF_ENABLED);
}

//*********************************
//* Setzt das zu verwaltende Men� *
//*********************************
void lhWinAPI::MenuDis::SetMenu(HMENU hMenu)
{
	this->hMenu = hMenu;
}

//**********************************************
//* F�gt einen zu verwaltenden Men�punkt hinzu *
//**********************************************
void lhWinAPI::MenuDis::AddPoint(int point)
{
	vec.push_back(point);
}

//**************************
//* L�scht alle Men�punkte *
//**************************
void lhWinAPI::MenuDis::Clear()
{
	vec.clear();
}

//*********************************************************************************
// Mapped ASCII String zu TCHAR string (gleiche Parameter wie MultiByteToWideChar() 
// wirft lhstd::runtime_error falls etwas schiefgeht
int lhWinAPI::MultiByteToWideChar(UINT CodePage, DWORD dwFlags, const char *szAsciiStr, int cbMultiByte, 
										 std::basic_string<wchar_t>& sWideCharStr)
{
	wchar_t *pBuffer = 0;
	int size = ::MultiByteToWideChar(CodePage, dwFlags, szAsciiStr, cbMultiByte, pBuffer, 0);
	if (size > 0)
	{
		pBuffer = new wchar_t[size+1];
		if (pBuffer)
		{
			size = ::MultiByteToWideChar(CodePage, dwFlags, szAsciiStr, cbMultiByte, pBuffer, size+1);
			pBuffer[size] = '\0';
			sWideCharStr = pBuffer;
			delete[] pBuffer;
		}
		else
			size = 0;
	}
	return size;
}

//*********************************************************************************
// Mapped ASCII String zu TCHAR string (gleiche Parameter wie MultiByteToWideChar() 
// wirft lhstd::runtime_error falls etwas schiefgeht
int lhWinAPI::MultiByteToWideChar(UINT CodePage, DWORD dwFlags, const std::basic_string<char>& sAsciiStr, 
										 std::basic_string<wchar_t>& sWideCharStr)
{
	return MultiByteToWideChar(CodePage, dwFlags, sAsciiStr.data(), static_cast<int>(sAsciiStr.length()), sWideCharStr);
}

//*********************************************************************************
// Mapped TCHAR String zu char string (gleiche Parameter wie WideCharToMultiByte() 
// wirft lhstd::runtime_error falls etwas schiefgeht
int lhWinAPI::WideCharToMultiByte(UINT CodePage, DWORD dwFlags, const wchar_t *sWideCharStr, int cchWideChar, std::string& sAsciiStr, 
								   const char *psDefaultChar, LPBOOL lpUsedDefaultChar)
{
	int size = ::WideCharToMultiByte(CodePage, dwFlags, sWideCharStr, cchWideChar, nullptr, 0, psDefaultChar, lpUsedDefaultChar);
	if (size > 0)
	{
		std::unique_ptr<char>pBuffer(new char[size+1]);
		if (pBuffer)
		{
			size = ::WideCharToMultiByte(CodePage, dwFlags, sWideCharStr, cchWideChar, pBuffer.get(), size+1, psDefaultChar, lpUsedDefaultChar);	
			pBuffer.get()[size] = '\0';
			sAsciiStr = pBuffer.get();
		}
		else
			size = 0;
	}
	return size;
}

//*********************************************************************************
// Mapped TCHAR String zu char string (gleiche Parameter wie WideCharToMultiByte() 
// wirft lhstd::runtime_error falls etwas schiefgeht
int lhWinAPI::WideCharToMultiByte(UINT CodePage, DWORD dwFlags, const std::wstring& sWideCharStr, std::string& sAsciiStr, 
								   const char *psDefaultChar, LPBOOL lpUsedDefaultChar)
{
	return WideCharToMultiByte(CodePage, dwFlags, sWideCharStr.data(), static_cast<int>(sWideCharStr.length()), sAsciiStr, 
								psDefaultChar, lpUsedDefaultChar);
}