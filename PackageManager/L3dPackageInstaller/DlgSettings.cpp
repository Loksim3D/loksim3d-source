#include "StdAfx.h"
#include "DlgSettings.h"
#include <KompexSQLiteException.h>

#include "DBHelper.h"

#include <Windowsx.h>
#include <lhWinAPI.h>
#include <string>
#include <boost/algorithm/string.hpp>

#include "Resource.h"
#include "RegistryAccess.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

using namespace db;
using namespace std;

DlgSettings::DlgSettings(void)
{
}


DlgSettings::~DlgSettings(void)
{
}

INT_PTR DlgSettings::Show(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_SETTINGS), hWndParent);
}


INT_PTR DlgSettings::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	SendDlgItemMessage(GetHwnd(), IDC_SET_IN_ASK, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_INSTALL_ALWAYS_ASK, false) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(GetHwnd(), IDC_SET_IN_ONLYOLDER, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_INSTALL_ONLY_OLDER, true) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(GetHwnd(), IDC_SET_UN_ASK, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_DEINSTALL_ALWAYS_ASK, false) ? BST_CHECKED : BST_UNCHECKED, 0);

	lhWinAPI::StringTable strTab;
	SendDlgItemMessage(GetHwnd(), IDC_SET_ALLOW_UNDO, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_ALLOW_UNDO, false) ? BST_CHECKED : BST_UNCHECKED, 0);

	HWND cb = GetDlgItem(GetHwnd(), IDC_COMBOLANG);
	ComboBox_AddString(cb, strTab.Load(IDS_LANG_DE));
	ComboBox_AddString(cb, strTab.Load(IDS_LANG_EN));

	wstring cul = RegistryAccess::GetUiCulture();
	if (cul.length() > 0)
	{
		if (boost::algorithm::iequals(cul, L"de-DE"))
		{
			ComboBox_SetCurSel(cb, 0);
		}
		else if (boost::algorithm::iequals(cul, L"en-US"))
		{
			ComboBox_SetCurSel(cb, 1);
		}
	}

	return Dialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgSettings::OnCommand(WPARAM wParam, LPARAM lParam)
{	//if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
	//{
	//	EndDialog(hDlg, LOWORD(wParam));
	//}
	switch(LOWORD(wParam))
	{
	case IDOK:
		try
		{
			DBHelper::instance().SetPreferenceValue(DBHelper::PREF_INSTALL_ALWAYS_ASK,  
				Button_GetCheck(GetDlgItem(GetHwnd(), IDC_SET_IN_ASK)) == BST_CHECKED);
			DBHelper::instance().SetPreferenceValue(DBHelper::PREF_INSTALL_ONLY_OLDER,  
				Button_GetCheck(GetDlgItem(GetHwnd(), IDC_SET_IN_ONLYOLDER)) == BST_CHECKED);
			DBHelper::instance().SetPreferenceValue(DBHelper::PREF_DEINSTALL_ALWAYS_ASK,  
				Button_GetCheck(GetDlgItem(GetHwnd(), IDC_SET_UN_ASK)) == BST_CHECKED);

			DBHelper::instance().SetPreferenceValue(DBHelper::PREF_ALLOW_UNDO,  
				Button_GetCheck(GetDlgItem(GetHwnd(), IDC_SET_ALLOW_UNDO)) == BST_CHECKED);

			int sel = ComboBox_GetCurSel(GetDlgItem(GetHwnd(), IDC_COMBOLANG));
			wchar_t *lang = nullptr;
			if (sel == 0)
			{
				lang = L"de-DE";
			}
			else if (sel == 1)
			{
				lang = L"en-US";
			}
			if (lang != nullptr)
			{
				RegistryAccess::SetUiCulture(lang);
			}

		} catch (const Kompex::SQLiteException&)
		{
			lhWinAPI::StringTable strTable;
			MessageBox(GetHwnd(), strTable.Load(IDS_SETTINGS_SAVE_ERROR), nullptr, MB_OK | MB_ICONERROR);
		}
		// fall through -> close dlg
	case IDCANCEL:
		EndDialog(GetHwnd(), LOWORD(wParam));
		return TRUE;
	default:
		return Dialog::OnCommand(wParam, lParam);
	}
	return Dialog::OnCommand(wParam, lParam);
}

}
}
}