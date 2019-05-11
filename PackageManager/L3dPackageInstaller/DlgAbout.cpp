#include "StdAfx.h"
#include "DlgAbout.h"

#include <string>
#include <lhWinAPI.h>

#include <Shellapi.h>

#include <algorithm>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

DlgAbout::DlgAbout(void)
{
}


DlgAbout::~DlgAbout(void)
{
}

INT_PTR DlgAbout::Show(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWndParent);
}

INT_PTR DlgAbout::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDOK:
	case IDCANCEL:
		EndDialog(GetHwnd(), 0);
		return TRUE;

	case IDC_ABOUT_KOMPEX:
		ShellExecute(GetHwnd(), NULL, L"http://sqlitewrapper.kompex-online.com/", NULL, NULL, SW_SHOW);  
		return TRUE;

	case IDC_ABOUT_SHA:
		ShellExecute(GetHwnd(), NULL, L"http://www.dominik-reichl.de/projects/csha1/", NULL, NULL, SW_SHOW);  
		return TRUE;

	case IDC_ABOUT_ZIP:
		ShellExecute(GetHwnd(), NULL, L"http://www.wischik.com/lu/programmer/zip_utils.html", NULL, NULL, SW_SHOW);  
		return TRUE;

	case IDC_ABOUT_LOKSIM:
		ShellExecute(GetHwnd(), NULL, L"https://www.loksim3d.de", NULL, NULL, SW_SHOW);
		return TRUE;
	}
	return FALSE;
}


/**
 * Hilfsfrozedur für den Info-Menüpunkt<br>
 * Anzeige HAnd beim Fahren über URLs
 */
static LRESULT CALLBACK UrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC proc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch(uMsg)
	{
	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return(TRUE);
	}
	return (CallWindowProc(proc, hWnd, uMsg, wParam, lParam));  
}

void DlgAbout::SetUrlCtrl(HWND urlCtrl)
{
	LOGFONT lf;
	HFONT hFont;

	if(nullptr == (hFont = (HFONT)SendMessage(urlCtrl, WM_GETFONT, 0, 0)))
	hFont = (HFONT)GetStockObject(SYSTEM_FONT);

	GetObject((HGDIOBJ)hFont, sizeof(lf), &lf);
	lf.lfUnderline = TRUE;
	hFont = CreateFontIndirect(&lf);

	SetWindowLongPtr(urlCtrl, GWLP_USERDATA, 
		SetWindowLongPtr(urlCtrl, GWLP_WNDPROC, (LONG_PTR)UrlProc));
	
	SendMessage(urlCtrl, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);

	fontsToDelete.push_back(hFont);
}

INT_PTR DlgAbout::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	TCHAR filePath[MAX_PATH];
	GetModuleFileName(nullptr, filePath, MAX_PATH);

	std::wstring fn = filePath;
	std::wstring vers;
	bool preRelease;
	lhWinAPI::GetFileVersion(fn, vers, &preRelease);

	SetDlgItemText(GetHwnd(), IDC_ABOUT_VERSION, vers.c_str());

	SetUrlCtrl(GetDlgItem(GetHwnd(), IDC_ABOUT_SHA));
	SetUrlCtrl(GetDlgItem(GetHwnd(), IDC_ABOUT_KOMPEX));
	SetUrlCtrl(GetDlgItem(GetHwnd(), IDC_ABOUT_ZIP));
	SetUrlCtrl(GetDlgItem(GetHwnd(), IDC_ABOUT_LOKSIM));

	return Dialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgAbout::DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NCDESTROY:
		std::for_each(begin(fontsToDelete), end(fontsToDelete), [](HFONT f) { DeleteObject((HGDIOBJ)f); });
		break;

	case WM_CTLCOLORSTATIC:
		if(	GetDlgItem(GetHwnd(), IDC_ABOUT_KOMPEX) == (HWND)lParam ||
			GetDlgItem(GetHwnd(), IDC_ABOUT_SHA) == (HWND)lParam ||
			GetDlgItem(GetHwnd(), IDC_ABOUT_LOKSIM) == (HWND)lParam ||
			GetDlgItem(GetHwnd(), IDC_ABOUT_ZIP) == (HWND)lParam)
		{
			SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHT));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return((BOOL)GetStockObject(NULL_BRUSH));
		}
		break;
	}
	return Dialog::DlgWndProc(message, wParam, lParam);
}

}
}
}
