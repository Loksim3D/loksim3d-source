#include "StdAfx.h"
#include "MainDlg.h"
#include <commctrl.h>

#include <Uxtheme.h>

#include "DlgDbUpdate.h"
#include "DlgSettings.h"
#include "DlgAbout.h"
#include "DBHelper.h"


namespace l3d
{
namespace packageinstaller
{
namespace ui
{

static int InsertTabItem(HWND hTab, LPTSTR pszText, int iid);
static HRESULT WINAPI MyEnableThemeDialogTexture(HWND hWnd, DWORD dwFlags);
typedef HRESULT (WINAPI * ENABLETHEMEDIALOGTEXTURE)(HWND, DWORD);

//using namespace std;

static const int TAB_ID_INSTALL = 0;
static const int TAB_ID_DEINSTALL = 1;
static const int TAB_ID_UNDO = 2;
static const int TAB_ID_DEPENDENCIES = 3;

MainDlg::MainDlg(const std::wstring& l3dPath, const std::vector<std::wstring>& packagesToInstall) 
	: dlgInstall(l3dPath, packagesToInstall), dlgDeinstall(l3dPath), dlgUndo(l3dPath), dlgDependencies(l3dPath), curTabDlg(nullptr), l3dPath(l3dPath)
{
	POINT min;
	min.x = 735;
	min.y = 450;
	SetMinSize(min);
}


MainDlg::~MainDlg(void)
{
}

INT_PTR MainDlg::Show(HINSTANCE hInstance, HWND hWndParent)
{
	return ResizeableDialog::Show(hInstance, MAKEINTRESOURCE(IDD_MAINPKGINSTALLER), hWndParent);
}

INT_PTR MainDlg::OnInitDlg(WPARAM wParam, LPARAM lParam)
{	
	// TabCtrl initialisieren
	HWND tabWindow = GetDlgItem(GetHwnd(), IDC_TABCTRLMAIN);
	HWND hTemp;

	SetDlgItemText(GetHwnd(), IDC_STATIC_INST_L3D_PATH, l3dPath.c_str());

	InsertTabItem(tabWindow, strTable.Load(IDS_TAB_HEADER_INSTALL), TAB_ID_INSTALL);
	InsertTabItem(tabWindow, strTable.Load(IDS_TAB_HEADER_DEINSTALL), TAB_ID_DEINSTALL);

	if (db::DBHelper::instance().GetPreferenceValue(db::DBHelper::PREF_ALLOW_UNDO, false))
	{
		InsertTabItem(tabWindow, strTable.Load(IDS_TAB_HEADER_UNDO), TAB_ID_UNDO);
		InsertTabItem(tabWindow, strTable.Load(IDS_TAB_HEADER_FILEDEPENDENCIES), TAB_ID_DEPENDENCIES);
	}

	hTemp = dlgDependencies.CreateModeless(GetHInstance(), GetHwnd());
	ShowWindow(hTemp, SW_HIDE);
	BringWindowToTop(hTemp);

	hTemp = dlgUndo.CreateModeless(GetHInstance(), GetHwnd());
	ShowWindow(hTemp, SW_HIDE);
	BringWindowToTop(hTemp);

	hTemp = dlgDeinstall.CreateModeless(GetHInstance(), GetHwnd());
	ShowWindow(hTemp, SW_HIDE);
	BringWindowToTop(hTemp);

	hTemp = dlgInstall.CreateModeless(GetHInstance(), GetHwnd());
	ShowWindow(hTemp, SW_SHOW);
	BringWindowToTop(hTemp);

	curTabDlg = &dlgInstall;

	// Dialoge in TabCtrls haben unter Windows XP ein
	// anderes Aussehen als Dialoge, die nicht in einem
	// TabCtrl sitzen. Hier wird die Version des Tabs
	// getestet und bei Bedarf die Dialog entsprechend
	// angepasst
	if(SendMessage(tabWindow, CCM_GETVERSION, 0, 0) >= 6)
	{
		 MyEnableThemeDialogTexture(dlgDeinstall.GetHwnd(), ETDT_ENABLETAB);
		 MyEnableThemeDialogTexture(dlgInstall.GetHwnd(), ETDT_ENABLETAB);
		 MyEnableThemeDialogTexture(dlgUndo.GetHwnd(), ETDT_ENABLETAB);
		 MyEnableThemeDialogTexture(dlgDependencies.GetHwnd(), ETDT_ENABLETAB);
	 }

	// Da sich der Dialog in der Groesse nicht veraendern laesst,
	// Kann die Groessenanpassung der Child-Dialoge bereits hier
	// erfolgen            
	RECT rc;
	GetWindowRect(tabWindow, &rc);
	ScreenToClient(GetHwnd(), (LPPOINT)&rc.left);
	ScreenToClient(GetHwnd(), (LPPOINT)&rc.right);

	SendMessage(tabWindow, TCM_ADJUSTRECT, (WPARAM)FALSE, (LPARAM)&rc);
	MoveWindow(dlgInstall.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(dlgDeinstall.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(dlgUndo.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(dlgDependencies.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	HICON hIcon = LoadIcon (GetHInstance(), MAKEINTRESOURCE (IDI_L3DPACKAGEINSTALLER));
	SendMessage (GetHwnd(), WM_SETICON, WPARAM (ICON_SMALL), LPARAM (hIcon));

	using l3d::packageinstaller::ui::resizeabledlghelper::ANCHOR;
	AddAnchor(ANCHOR(IDC_TABCTRLMAIN,			resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_TOP_AND_BOTTOM));
	AddAnchor(ANCHOR(IDC_STATICL3DPATH,			resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));
	AddAnchor(ANCHOR(IDC_STATIC_INST_L3D_PATH,	resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));

	//AddAnchor(ANCHOR(IDD_INSTALLPACKAGES,		resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_TOP_AND_BOTTOM));


	try {
		if (db::DBHelper::instance().IsDbUpdateRequired()) {
			DlgDbUpdate dlg{ l3dPath };
			dlg.Show(GetHInstance(), GetHwnd());
		}
	}
	catch (...) {
	}

	return ResizeableDialog::OnInitDlg(wParam, lParam);
}

INT_PTR MainDlg::OnSize(WPARAM wParam, LPARAM lParam)
{
	HWND tabWindow = GetDlgItem(GetHwnd(), IDC_TABCTRLMAIN);
	RECT rc;
	GetWindowRect(tabWindow, &rc);
	ScreenToClient(GetHwnd(), (LPPOINT)&rc.left);
	ScreenToClient(GetHwnd(), (LPPOINT)&rc.right);

	SendMessage(tabWindow, TCM_ADJUSTRECT, (WPARAM)FALSE, (LPARAM)&rc);
	MoveWindow(dlgInstall.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(dlgDeinstall.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(dlgUndo.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	MoveWindow(dlgDependencies.GetHwnd(), rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	return ResizeableDialog::OnSize(wParam, lParam);
}

INT_PTR MainDlg::OnClose(WPARAM wParam, LPARAM lParam)
{
	if (curTabDlg == nullptr || curTabDlg->SoftCancel())
	{
		EndDialog(GetHwnd(), 0);
	}
	return TRUE;
}

INT_PTR MainDlg::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return TRUE;
}

INT_PTR MainDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	wmId    = LOWORD(wParam);
	wmEvent = HIWORD(wParam);
	// Parse the menu selections:
	switch (wmId)
	{
	case IDM_ABOUT:
		{
			DlgAbout dlg;
			dlg.Show(GetHInstance(), GetHwnd());
		}
		return TRUE;
	case IDM_EXIT:
		OnClose(wParam, lParam);
		return TRUE;
	case IDM_SETTINGS:
		{
			bool allowUndoBefore = db::DBHelper::instance().GetPreferenceValue(db::DBHelper::PREF_ALLOW_UNDO, false);

			DlgSettings dlg;
			dlg.Show(GetHInstance(), GetHwnd());
			
			bool allowUndoAfter = db::DBHelper::instance().GetPreferenceValue(db::DBHelper::PREF_ALLOW_UNDO, false);
			if (allowUndoAfter != allowUndoBefore)
			{
				HWND tabWindow = GetDlgItem(GetHwnd(), IDC_TABCTRLMAIN);
				int ind = TabCtrl_GetCurSel(tabWindow);
				if (allowUndoAfter)
				{
					InsertTabItem(tabWindow, strTable.Load(IDS_TAB_HEADER_UNDO), TAB_ID_UNDO);
					InsertTabItem(tabWindow, strTable.Load(IDS_TAB_HEADER_FILEDEPENDENCIES), TAB_ID_DEPENDENCIES);
				}
				else
				{
					if (TAB_ID_UNDO == ind || TAB_ID_DEPENDENCIES == ind) {
						ShowWindow(curTabDlg->GetHwnd(), SW_HIDE);
						curTabDlg = &dlgInstall;
						TabCtrl_SetCurSel(tabWindow, TAB_ID_INSTALL);
						ShowWindow(curTabDlg->GetHwnd(), SW_SHOW);
						BringWindowToTop(curTabDlg->GetHwnd());
					}
					TabCtrl_DeleteItem(tabWindow, TAB_ID_DEPENDENCIES);
					TabCtrl_DeleteItem(tabWindow, TAB_ID_UNDO);
				}
			}

		}
		return TRUE;
	}
	return ResizeableDialog::OnCommand(wParam, lParam);
}

INT_PTR MainDlg::OnNotify(WPARAM wParam, LPARAM lParam)
{
	switch (((LPNMHDR)lParam)->code)
	{
	case TCN_SELCHANGING:
		if (curTabDlg != nullptr)
		{
			if (curTabDlg->IsOperationRunning())
			{
				SetWindowLongPtr(GetHwnd(), DWLP_MSGRESULT, TRUE);
				return TRUE;
			}
			else
			{
				ShowWindow(curTabDlg->GetHwnd(), SW_HIDE);
			}
		}
		break;
	case TCN_SELCHANGE:
		{
			int ind = TabCtrl_GetCurSel(GetDlgItem(GetHwnd(), IDC_TABCTRLMAIN));
			switch (ind)
			{
			case TAB_ID_INSTALL:
				curTabDlg = &dlgInstall;
				break;
			case TAB_ID_DEINSTALL:
				curTabDlg = &dlgDeinstall;
				break;
			case TAB_ID_UNDO:
				curTabDlg = &dlgUndo;
				break;
			case TAB_ID_DEPENDENCIES:
				curTabDlg = &dlgDependencies;
				break;
			}
			if (curTabDlg != nullptr)
			{
				ShowWindow(curTabDlg->GetHwnd(), SW_SHOW);
				BringWindowToTop(curTabDlg->GetHwnd());
			}
		}
		break;
	}
	return ResizeableDialog::OnNotify(wParam, lParam);
}

static int InsertTabItem(HWND hTab, LPTSTR pszText, int iid)
{
	TCITEM ti = {0};
	ti.mask = TCIF_TEXT;
	ti.pszText = pszText;
	ti.cchTextMax = wcslen(pszText);

	return (int)SendMessage(hTab, TCM_INSERTITEM, iid, (LPARAM)&ti);
}



// Hier werden die Dialoge für das TabCtrl vorbereitet. Noetig ist
// das nur unter XP. Aber da das Programm unter alenn aktuellen
// Plattformen vernuenftig laufen soll, habe ich das hier mit
// aufgenommen.
static HRESULT WINAPI MyEnableThemeDialogTexture(HWND hWnd, DWORD dwFlags)
{
  ENABLETHEMEDIALOGTEXTURE pfnETDT;
  HINSTANCE                hDll;
  HRESULT                  hr;

	hr = HRESULT_FROM_WIN32(ERROR_CALL_NOT_IMPLEMENTED);

	if(NULL != (hDll = LoadLibrary(TEXT("uxtheme.dll"))))
	{
		if(NULL != (pfnETDT = (ENABLETHEMEDIALOGTEXTURE)GetProcAddress(hDll, "EnableThemeDialogTexture")))
		{
			hr = pfnETDT(hWnd, dwFlags);
		}
		FreeLibrary(hDll);
	}

  return(hr);
}

}
}
}