#include "..\Include\lhWinAPIW.h"
#include "..\Include\lhWinAPIG.h"
#include <shlobj.h>
#include <objbase.h>
#include <objidl.h>
#include <sstream>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 

#include <regex>

using namespace std;

#if defined(UNICODE) || defined(_UNICODE)

//****************************************
//* Fügt Element zu Treeview hwndTV hinzu
//****************************************
HTREEITEM lhWinAPI::AddItemToTree(HWND hwndTV, std::basic_string<wchar_t> nameItem, int nLevel, LPARAM lParam, int image)
{ 
	wchar_t *lpszItem = new wchar_t[nameItem.length() + 1];
	TVITEMEXW tvi = {0}; 
	TVINSERTSTRUCTW tvins; 
	static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST; 
	static HTREEITEM hPrevRootItem = NULL; 
	static HTREEITEM hPrevLev2Item = NULL; 
	static HTREEITEM hPrevLev3Item = NULL; 
	HTREEITEM hti; 
	
	wcscpy_s(lpszItem, nameItem.length() + 1, nameItem.c_str());

	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE; 

	// Set the text of the item. 
	tvi.pszText = lpszItem; 
	tvi.cchTextMax = nameItem.length() + 1; 
	tvi.iImage = image;
	tvi.iSelectedImage = image;

	// Save the heading level in the item's application-defined 
	// data area. 
	tvi.lParam = (LPARAM)lParam; 
	tvins.itemex = tvi; 
	tvins.hInsertAfter = hPrev; 


	// Set the parent item based on the specified level. 
	if (nLevel == 1) 
		tvins.hParent = TVI_ROOT; 
	else if (nLevel == 2) 
		tvins.hParent = hPrevRootItem; 
	else if (nLevel == 3)
		tvins.hParent = hPrevLev2Item; 
	else
		tvins.hParent = hPrevLev3Item; 

	// Add the item to the tree-view control. 
	hPrev = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM, 0, (LPARAM)(LPTVINSERTSTRUCT)&tvins); 

	// Save the handle to the item. 
	if (nLevel == 1) 
		hPrevRootItem = hPrev; 
	else if (nLevel == 2) 
		hPrevLev2Item = hPrev; 
	else if (nLevel == 3) 
		hPrevLev3Item = hPrev; 

	// The new item is a child item. Give the parent item a 
	// closed folder bitmap to indicate it now has child items. 
	if (nLevel > 1)
	{ 
		hti = TreeView_GetParent(hwndTV, hPrev); 
		tvi.mask = TVIF_HANDLE; 
		tvi.hItem = hti; 
		TreeView_SetItem(hwndTV, &tvi); 
	} 
	delete [] lpszItem;
	return hPrev; 
}

//*********************************************
//* Suche nach Ordner (BrowseForFolder Dialog)
//*********************************************
std::basic_string<wchar_t> lhWinAPI::BrowseForFolder(HWND hwnd, std::basic_string<wchar_t> title)
{
	BROWSEINFOW bi;
	wchar_t szPath[MAX_PATH + 1];
	LPITEMIDLIST pidl;
	BOOL bResult = FALSE;
	std::basic_string<wchar_t> ret;
	LPMALLOC pMalloc;

	if (SUCCEEDED(SHGetMalloc(&pMalloc))) 
	{
		bi.hwndOwner = hwnd;
		bi.pidlRoot = NULL;
		bi.pszDisplayName = NULL;
		bi.lpszTitle = title.c_str();
		bi.ulFlags = BIF_STATUSTEXT; //BIF_EDITBOX 
		bi.lpfn = NULL;
		bi.lParam = (LPARAM)szPath;
		
		pidl = SHBrowseForFolderW(&bi);
		if (pidl)
		{
			if (SHGetPathFromIDListW(pidl,szPath)) 
			{
				bResult = TRUE;
				ret = szPath;
			}
		   pMalloc->Free(pidl);
		   pMalloc->Release();            			
		}
	}

	return bResult ? ret : L"";	
}

//****************************************************
//* Liefert den letzten Fehler mittels GetLastError() 
std::wstring lhWinAPI::GetLastErrorS()
{
	return GetLastErrorS(GetLastError());
}

//****************************************************
//* Liefert eine Fehlermeldung auf Basis von dwError 
std::wstring lhWinAPI::GetLastErrorS(DWORD dwError)
{
	LPWSTR lpMsgBuf;

	std::basic_string<wchar_t> s;
	if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,  0, dwError, 
										0/*MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)*/, (LPWSTR) &lpMsgBuf, 0, NULL))
	{
		std::basic_stringstream<TCHAR> str;
		str << TEXT("Unbekannter Fehler - Fehlercode: ") << dwError;
		s = str.str();
	}
	else
		s = static_cast<wchar_t*>(lpMsgBuf);	
	
	if (lpMsgBuf != 0)
		LocalFree(lpMsgBuf);
	return s;
}

//*************************************************************
//* Liefert den absoluten Pfad von fileName im EXE Verzeichnis 
//*************************************************************
std::basic_string<wchar_t> lhWinAPI::FilePathExec(std::basic_string<wchar_t> fileName)
{
	wchar_t buffer[MAX_PATH];
	wchar_t szPath[MAX_PATH];
	wchar_t *pointer = buffer;

	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	GetFullPathNameW(buffer, MAX_PATH, szPath, &pointer);
	*pointer = '\0';
	return szPath + fileName;
}

/*******************************************************
 *******************************************************
 * Class StringTable	(Strings in versch. Sprachen   *	
 *******************************************************
 *******************************************************/

/***********************************************
 * Liefert einen TCHAR* auf Message 		   *
 ***********************************************/
wchar_t* lhWinAPI::StringTable::Load(UINT msg)
{
	buffer[0] = '\0';
	LoadStringW(GetModuleHandle(NULL), msg, buffer, sizeof buffer / sizeof buffer[0]);
	return buffer;
}

/*************************************************
 * Liefert einen std::basic_string<> auf Message *
 *************************************************/
std::basic_string<wchar_t> lhWinAPI::StringTable::LoadS(UINT msg)
{
	buffer[0] = '\0';
	LoadStringW(GetModuleHandle(NULL), msg, buffer, sizeof buffer / sizeof buffer[0]);
	return buffer;
}

//***************************************
//* Liest string aus WindowText hwnd aus 
//***************************************
std::wstring lhWinAPI::GetWindowString(HWND hwnd)
{
	wchar_t szBuffer[1024] = { 0 };

	GetWindowTextW(hwnd, szBuffer, 1023);
	std::wstring s(szBuffer);

	return s;
}

//**************************************************************************************************
// Konvertiert den TCHAR-String src in den char-String dest; Liefert Anzahl umgewandelter Zeichen zurück
int lhWinAPI::TcharToChar(std::string& dest, const std::wstring& src)
{
	return WideCharToMultiByte(0, 0, src, dest, 0, 0);
}

//**************************************************************************************************
// Konvertiert den Char-String src in den TCHAR-String dest; Liefert Anzahl umgewandelter Zeichen zurück	
int lhWinAPI::CharToTchar(std::wstring& dest, const std::string& src)
{
	return MultiByteToWideChar(0, 0, src, dest);
}

//**************************************************************************************************
// Konvertiert den Wchar-String src in den TCHAR-String dest; Liefert Anzahl umgewandelter Zeichen zurück	
int lhWinAPI::WcharToTchar(std::wstring& dest, const std::wstring& src)
{
	dest = src;
	return static_cast<int>(dest.length());
}

//**************************************************************************************************
// Konvertiert den Tchar-String src in den Wchar-String dest; Liefert Anzahl umgewandelter Zeichen zurück	
int lhWinAPI::TcharToWchar(std::wstring& dest, const std::wstring& src)
{
	dest = src;
	return static_cast<int>(dest.length());
}

//********************************************
//* Testet ob die Datei fileName existiert
bool lhWinAPI::FileExists(const std::wstring& fileName)
{
	WIN32_FIND_DATAW findData = { 0 };
	
	StringCbCopyW(findData.cFileName, 256, fileName.c_str());
	HANDLE h = FindFirstFileW(fileName.c_str(), &findData);
	if (h == INVALID_HANDLE_VALUE)
		return false;
	FindClose(h);
	return true;
}

//**************************************************************************************************************
//* Speichert die Version der Datei sFileName auf dest, isPreRelease wird true falls die Version PreRelease ist 
// liefert true falls keine Fehler aufgetreten sind
struct LANGANDCODEPAGE 
{
	WORD wLanguage;
	WORD wCodePage;
};

typedef DWORD (__stdcall *GETFILEVERSIONINFOSIZE) (LPCTSTR, LPDWORD);



bool lhWinAPI::GetFileVersion(std::wstring& sFileName, std::wstring& dest, bool* isPreRelease)
{
	DWORD dwSize, dwHandle;
	struct LANGANDCODEPAGE *lpTranslate = 0;

	HMODULE hDll = LoadLibrary(TEXT("version.dll"));
	if (hDll == NULL)
	{
		return false;
	//	throw lhstd::runtime_error(GetLastErrorS());
	}

	GETFILEVERSIONINFOSIZE hGetFileVersionInfoSize = (GETFILEVERSIONINFOSIZE)GetProcAddress(hDll, "GetFileVersionInfoSizeW");

	//GetModuleFileName(NULL, fileName, 512);
	
	if (!(dwSize = GetFileVersionInfoSize(sFileName.c_str(), &dwHandle)))
	{
		FreeLibrary(hDll);
		return false;
	}
	BYTE *pData = new BYTE[dwSize];
	if (!GetFileVersionInfo(sFileName.c_str(), dwHandle, dwSize, static_cast<void*>(pData)))
	{
		FreeLibrary(hDll);
		return false;
	}
	UINT uLen;
	HRESULT hr;
	TCHAR SubBlock[50];

	// Read the list of languages and code pages.
	if (!VerQueryValue(static_cast<void*>(pData), TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &uLen))
	{
		FreeLibrary(hDll);
		return false;
	}
	TCHAR* szFileVersion = { 0 };
	bool bWritten = false;
	for(UINT i=0; i < (uLen/sizeof(struct LANGANDCODEPAGE)); i++ )
	{
		UINT uBytes;			
		hr = StringCchPrintf(SubBlock, 50, TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"), 
							 lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
		if (FAILED(hr))
			continue;
		// Retrieve file description for language and code page "i". 
		if (VerQueryValue(static_cast<void*>(pData), SubBlock, reinterpret_cast<LPVOID*>(&szFileVersion), &uBytes))
		{
			bWritten = true;
			break;
		}
	}

	VS_FIXEDFILEINFO *pvsFixed = 0;
	if (VerQueryValue(static_cast<void*>(pData), TEXT("\\"), reinterpret_cast<LPVOID*>(&pvsFixed), &uLen))
	{
		if ((pvsFixed->dwFileFlags & VS_FF_PRERELEASE) && isPreRelease != 0 )
			*isPreRelease = true;
	}

	wstring v = szFileVersion;
	wregex rx(L"\\s");    
	wstring replacement = L"";   
	v = regex_replace(v, rx, replacement);
	rx = (L",");    
	replacement = L".";   
	v = regex_replace(v, rx, replacement);

	dest += v;
	FreeLibrary(hDll);
	return bWritten;
}

#endif
