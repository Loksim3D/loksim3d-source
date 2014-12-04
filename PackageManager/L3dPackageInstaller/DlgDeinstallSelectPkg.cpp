#include "StdAfx.h"
#include "DlgDeinstallSelectPkg.h"

#include "PackageSearch.h"
#include "L3dPackageInstaller.h"
#include "TimeUtils.h"
#include "DlgDeInstallSelectFiles.h"

#include <boost/algorithm/string.hpp>  
#include <algorithm>
#include <Commctrl.h>
#include <strsafe.h>

#include "DBHelper.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

using namespace std;
using namespace l3d::packageinstaller::db;

DlgDeinstallSelectPkg::DlgDeinstallSelectPkg(const boost::filesystem::path& l3dPath) : l3dPath(l3dPath), sortColDir(0, false), gatheredFiles(false), pkgIdToNameMap(make_shared<map<int, wstring>>())
{
}


DlgDeinstallSelectPkg::~DlgDeinstallSelectPkg(void)
{
}

//INT_PTR DlgDeinstallSelectPkg::Show(HINSTANCE hInstance, HWND hWndParent)
//{
//	return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_DEINSTALLPKGSEARCH), hWndParent);
//}

HWND DlgDeinstallSelectPkg::CreateModeless(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::CreateModeless(hInstance, MAKEINTRESOURCE(IDD_DEINSTALLPKGSEARCH), hWndParent);
}

bool DlgDeinstallSelectPkg::InitListViewColumns() 
{ 
	LVCOLUMN lvc;
	int iCol;

	// Initialize the LVCOLUMN structure.
	// The mask specifies that the format, width, text,
	// and subitem members of the structure are valid.
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	// Add the columns.
	for (iCol = 0; iCol < 2; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = strTable.Load(iCol == 0 ? IDS_PACKAGENAME : IDS_INSTALLEDON);
		lvc.cx = iCol == 0 ? 150 : 120;               // Width of column in pixels.

		lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.

		// Insert the columns into the list view.
		if (ListView_InsertColumn(hListSelectablePkgs, iCol, &lvc) == -1)
			return false;
	}
	
	return true;
}

void DlgDeinstallSelectPkg::ResetDialog()
{
	gatheredFiles = false;
	deinstallManager.reset(nullptr);
	pkgIdToNameMap->clear();
	ListView_DeleteAllItems(hListSelectedPkgs);
	selectedItems.clear();

	EnableWindow(hListSelectablePkgs, TRUE);
	EnableWindow(hListSelectedPkgs, TRUE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXASKBEFOREDEINSTALL), TRUE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELDEINSTALL), FALSE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONSTARTDEINSTALL), FALSE);

	SetDlgItemText(GetHwnd(), IDC_EDITREADME, L"");

	SendDlgItemMessage(GetHwnd(), IDC_PROGRESSDEINSTALL, PBM_SETPOS, 0, 0);
	SetDlgItemText(GetHwnd(), IDC_STATICPROGRESSDEINSTALL, L"");

	FilterSelectableItems();
}

void DlgDeinstallSelectPkg::SortSelectableItems()
{
	sort(begin(selectableItems), end(selectableItems), [this](const DBPackageEntry& en1, const DBPackageEntry& en2) -> int
	{
		if (sortColDir.first == 0)
		{
			if (sortColDir.second)
			{
				return en1.GetPackageName() > en2.GetPackageName();
			}
			return en1.GetPackageName() < en2.GetPackageName();
		}
		int64 timeDif = en1.GetInstalledTimestamp() - en2.GetInstalledTimestamp();
		if (sortColDir.second)
		{
			return timeDif == 0 ? 0 : (timeDif > 0 ? 1 : 0);
		}
		return timeDif == 0 ? 0 : (timeDif > 0 ? 0 : 1);
	});
	InsertListViewItems(hListSelectablePkgs, selectableItems.size());
}

void DlgDeinstallSelectPkg::SetSortHeader()
{
	HWND hHeader = ListView_GetHeader(hListSelectablePkgs);
	HDITEM hdi = { 0 };
	
	for (int iCol = 0; iCol < 2; ++iCol)
	{
		hdi.mask = HDI_FORMAT;
		Header_GetItem(hHeader, iCol, (LPHDITEM) &hdi);
		hdi.mask = HDI_FORMAT;
		hdi.fmt &= ~HDF_SORTDOWN & ~HDF_SORTUP;
		if (iCol == sortColDir.first)
		{
			hdi.fmt |= (sortColDir.second ? HDF_SORTDOWN : HDF_SORTUP);
		}
		Header_SetItem(hHeader, iCol, &hdi);
	}
}

bool DlgDeinstallSelectPkg::InsertListViewItems(HWND listView, size_t cntItems)
{
	LVITEM lvI;

	ListView_DeleteAllItems(listView);

	// Initialize LVITEM members that are common to all items.
	lvI.pszText   = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
	lvI.mask      = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem  = 0;
	lvI.state     = 0;

	// Initialize LVITEM members that are different for each item.
	for (size_t index = 0; index < cntItems; index++)
	{
		lvI.iItem  = index;
		lvI.iImage = index;
	
		// Insert items into the list.
		if (ListView_InsertItem(listView, &lvI) == -1)
			return FALSE;
	}

	return TRUE;
}


void DlgDeinstallSelectPkg::FilterSelectableItems()
{
	std::wstring filterText = L"%";
	filterText.append(lhWinAPI::GetWindowString(GetDlgItem(GetHwnd(), IDC_EDITFILTER)));
	filterText.append(L"%");
	if (filterText.length() > 2)
	{
		boost::to_lower(filterText);
		selectableItems = pkgSearch.SearchByText(filterText);
	}
	else
	{
		selectableItems = pkgSearch.GetAllPackages();
	}
	selectableItems.erase(remove_if(begin(selectableItems), end(selectableItems), [this](const DBPackageEntry& en) {
		return find_if(begin(selectedItems), end(selectedItems), [&en](const DBPackageEntry& innerEn) {
			return innerEn.GetId() == en.GetId();
		}) != end(selectedItems);
	}), end(selectableItems));
	SortSelectableItems();
}

bool DlgDeinstallSelectPkg::IsOperationRunning() const
{
	if (deinstallManager)
	{
		DeinstallManager::DeinstallState st = deinstallManager->GetState();
		return st == DeinstallManager::DeinstallGatheringFiles || st == DeinstallManager::DeinstallRunning; 
	}
	return false;
}

bool DlgDeinstallSelectPkg::SoftCancel()
{
	if (!deinstallManager)
	{
		//EndDialog(GetHwnd(), IDOK);
		//TODO cancel
		ResetDialog();
		return true;
	}
	else
	{
		DeinstallManager::DeinstallState st = deinstallManager->GetState();
		switch (st)
		{
		case DeinstallManager::DeinstallCancelled:
		case DeinstallManager::DeinstallError:
		case DeinstallManager::DeinstallFinished:
		case DeinstallManager::DeinstallGatheringFiles:
		case DeinstallManager::DeinstallGatheringFinished:
		case DeinstallManager::DeinstallNotRunning:
			ResetDialog();
			break;
		case DeinstallManager::DeinstallRunning:
			{
				bool trans = deinstallManager->IsUsingTransactions();
				if (deinstallManager->GetState() == DeinstallManager::DeinstallRunning && (
					trans || MessageBox(GetHwnd(), strTable.LoadS(IDS_WARNINGDEINSTALLCANCEL).c_str(), 
					strTable.LoadS(IDS_WARNINGDEINSTALLCANCELTITLE).c_str(), MB_ICONWARNING | MB_YESNO) == IDYES))
				{
					deinstallManager->CancelDeinstall();
					EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELDEINSTALL), FALSE);
				}
				else
				{
					return false;
				}
			}
		}
	}
	return true;
}

INT_PTR DlgDeinstallSelectPkg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (HIWORD(wParam) == EN_CHANGE)
	{
		FilterSelectableItems();
	}
	else if (LOWORD(wParam) == IDC_BUTTONCANCELDEINSTALL)
	{
		SoftCancel();
	}
	else if (LOWORD(wParam) == IDC_BUTTONSTARTDEINSTALL)
	{
		if (!deinstallManager)
		{
			EnableWindow(hListSelectablePkgs, FALSE);
			EnableWindow(hListSelectedPkgs, FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXASKBEFOREDEINSTALL), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELDEINSTALL), TRUE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONSTARTDEINSTALL), FALSE);
			vector<int> vecIds;
			transform(begin(selectedItems), end(selectedItems), back_inserter(vecIds), [](const DBPackageEntry& en) {
				return en.GetId();
			});

			for_each(begin(selectedItems),end(selectedItems), [this](const DBPackageEntry& en) {
				(*pkgIdToNameMap)[en.GetId()] = en.GetPackageName();
			});

			deinstallManager.reset(new DeinstallManager(l3dPath, move(vecIds)));
			if (IsDlgButtonChecked(GetHwnd(), IDC_CHECKBOXASKBEFOREDEINSTALL) == BST_CHECKED)
			{
				gatheredFiles = true;
				deinstallManager->StartGetFilesToDelete();
			}
			else
			{
				deinstallManager->StartDeinstall();
			}
			SetTimer(GetHwnd(), 0, 500, nullptr);
		}
	}
	return Dialog::OnCommand(wParam, lParam);
}

INT_PTR DlgDeinstallSelectPkg::OnTimer(WPARAM wParam, LPARAM lParam)
{
	if (deinstallManager)
	{
		size_t cur, total;
		SendDlgItemMessage(GetHwnd(), IDC_PROGRESSDEINSTALL, PBM_SETPOS, deinstallManager->GetProgress(&cur, &total), 0);
		wchar_t buf[2048];
		swprintf_s(buf, strTable.Load(IDS_INSTALLPROGTXT), cur, total);
		SetDlgItemText(GetHwnd(), IDC_STATICPROGRESSDEINSTALL, buf);

		if (deinstallManager->IsFinished())
		{
			KillTimer(GetHwnd(), 0);

			if (deinstallManager->GetState() == DeinstallManager::DeinstallGatheringFinished)
			{
				DlgDeInstallSelectFiles dlg(deinstallManager->GetFilesToDeinstall(), pkgIdToNameMap);
				if (dlg.Show(GetHInstance(), GetHwnd()) == IDOK)
				{
					deinstallManager->StartDeinstall(dlg.GetSelectedFiles());
					SetTimer(GetHwnd(), 0, 750, nullptr);
					return Dialog::OnTimer(wParam, lParam);
				}
			}
			else if (deinstallManager->GetState() == DeinstallManager::DeinstallFinished)
			{
				if (gatheredFiles)
				{
					std::wstring header = strTable.LoadS(IDS_DEINSTALLSUCCESSHEADER);
					MessageBox(GetHwnd(), strTable.Load(IDS_DEINSTALLSUCCESSTXT), header.c_str(), MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					DlgDeInstallSelectFiles dlg(deinstallManager->GetDeinstalledFiles(), pkgIdToNameMap);
					dlg.Show(GetHInstance(), GetHwnd());
				}
			}
			else if (deinstallManager->GetState() == DeinstallManager::DeinstallError)
			{
				DeinstallException *ex = deinstallManager->GetDeinstallError();
				if (ex != nullptr)
				{
					if (!ex->GetProcessedFilename().empty())
					{
						swprintf_s(buf, strTable.Load(IDS_DEINSTALLERRORFILETXT), ex->GetProcessedFilename().c_str(), ex->GetMsg().c_str());
					}
					else
					{
						std::wstring st = strTable.LoadS(IDS_DEINSTALLERRORTXT);
						swprintf_s(buf, st.c_str(), ex->GetMsg().c_str());
					}
				}
				else
				{
					wcscpy_s(buf, strTable.Load(IDS_DEINSTALLERRORUNKWOWN));
				}
				MessageBox(GetHwnd(), buf, strTable.Load(IDS_DEINSTALLERRORHEADER), MB_OK | MB_ICONERROR);
			}
			else if (deinstallManager->GetState() == DeinstallManager::DeinstallCancelled)
			{
				std::wstring msg = strTable.LoadS(IDS_INSTALLCANCELLED).append(L"\n");
				if (deinstallManager->IsUsingTransactions())
				{
					msg.append(strTable.LoadS(IDS_INSTALLROLLEDBACK));
					MessageBox(GetHwnd(), msg.c_str(), strTable.Load(IDS_DEINSTALLCANCELLDTITLE), MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					msg.append(strTable.LoadS(IDS_INSTALLPARTIAL));
					MessageBox(GetHwnd(), msg.c_str(), strTable.Load(IDS_DEINSTALLCANCELLDTITLE), MB_OK | MB_ICONWARNING);
				}
			}
			//EndDialog(GetHwnd(), IDOK);
			ResetDialog();
		}
	}
	return Dialog::OnTimer(wParam, lParam);
}

INT_PTR DlgDeinstallSelectPkg::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	hListSelectablePkgs = GetDlgItem(GetHwnd(), IDC_LISTALLPACKAGES);
	InitListViewColumns();
	hListSelectedPkgs = GetDlgItem(GetHwnd(), IDC_LISTDEINSTALLSELECTEDPKGS);

	selectableItems = pkgSearch.GetAllPackages();
	InsertListViewItems(hListSelectablePkgs, selectableItems.size());
	SetSortHeader();

	
	SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXASKBEFOREDEINSTALL, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_DEINSTALL_ALWAYS_ASK, false) ? BST_CHECKED : BST_UNCHECKED, 0);

	using l3d::packageinstaller::ui::resizeabledlghelper::ANCHOR;
	AddAnchor(ANCHOR(IDC_STATICDEINSTALLLEFT,			resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_STATICDEINSTALLRIGHTTOP,		resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_RIGHT));
	AddAnchor(ANCHOR(IDC_STATICDEINSTALLRIGHTBOTTOM,	resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_RIGHT));

	AddAnchor(ANCHOR(IDC_STATICFILTER,	resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_EDITFILTER,	resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT_AND_RIGHT));

	AddAnchor(ANCHOR(IDC_LISTALLPACKAGES,			resizeabledlghelper::AF_TOP_AND_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_LISTDEINSTALLSELECTEDPKGS,	resizeabledlghelper::AF_TOP_AND_BOTTOM | resizeabledlghelper::AF_RIGHT));

	AddAnchor(ANCHOR(IDC_STATICREADME,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_EDITREADME,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT_AND_RIGHT));


	AddAnchor(ANCHOR(IDC_STATICCHECKSUM, resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_EDITCHECKSUM, resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));


	AddAnchor(ANCHOR(IDC_PROGRESSDEINSTALL,		resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT_AND_RIGHT));

	AddAnchor(ANCHOR(IDC_STATICPROGRESSDEINSTALL,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_CHECKBOXASKBEFOREDEINSTALL,resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));

	AddAnchor(ANCHOR(IDC_BUTTONCANCELDEINSTALL,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_RIGHT));
	AddAnchor(ANCHOR(IDC_BUTTONSTARTDEINSTALL,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_RIGHT));

	return ResizeableDialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgDeinstallSelectPkg::OnSize(WPARAM wParam, LPARAM lParam)
{
	return ResizeableDialog::OnSize(wParam, lParam);
}

INT_PTR DlgDeinstallSelectPkg::OnNotify(WPARAM wParam, LPARAM lParam)
{
	static HWND focusedListView;

	std::wstring str;

	switch (((LPNMHDR) lParam)->code)
	{
	case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam;	
			size_t iItem = static_cast<size_t>(plvdi->item.iItem);
			if (plvdi->hdr.hwndFrom == hListSelectablePkgs && iItem < selectableItems.size())
			{
				switch (plvdi->item.iSubItem)
				{
				case 0:
					StringCchCopy(	plvdi->item.pszText, 
									plvdi->item.cchTextMax,
									selectableItems[iItem].GetPackageName().c_str());
					break;
					  
				case 1:
					str = FormatUnixTime(selectableItems[iItem].GetInstalledTimestamp());
					StringCchCopy(	plvdi->item.pszText, 
									plvdi->item.cchTextMax,
									str.c_str());
					break;                               
				default:
					break;
				}
			}
			else if (plvdi->hdr.hwndFrom == hListSelectedPkgs && iItem < selectedItems.size())
			{
				StringCchCopy(	plvdi->item.pszText, 
								plvdi->item.cchTextMax,
								selectedItems[iItem].GetPackageName().c_str());
			}
		}
		break;

	case NM_SETFOCUS:
		{
			LPNMHDR lpnmh = (LPNMHDR) lParam; 
			focusedListView = lpnmh->hwndFrom;
		}
		break;

	case LVN_ITEMCHANGED:
	case NM_CLICK:
		{
			int iItem = ListView_GetNextItem(focusedListView, -1, LVNI_SELECTED);
			if (iItem >= 0 && focusedListView == hListSelectablePkgs && static_cast<size_t>(iItem) < selectableItems.size())
			{
				lhWinAPI::SetWindowValue(GetDlgItem(GetHwnd(), IDC_EDITREADME), selectableItems[iItem].GetReadme());
				lhWinAPI::SetWindowValue(GetDlgItem(GetHwnd(), IDC_EDITCHECKSUM), selectableItems[iItem].GetChecksum());
			}
			else if (iItem >= 0 && focusedListView == hListSelectedPkgs && static_cast<size_t>(iItem) < selectedItems.size())
			{
				lhWinAPI::SetWindowValue(GetDlgItem(GetHwnd(), IDC_EDITREADME), selectedItems[iItem].GetReadme());
				lhWinAPI::SetWindowValue(GetDlgItem(GetHwnd(), IDC_EDITCHECKSUM), selectableItems[iItem].GetChecksum());
			}
			else
			{
				SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITREADME), L"");
				SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITCHECKSUM), L"");
			}
		}
		break;

	case LVN_COLUMNCLICK:
		{
			LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam; 
			if (pnmv->hdr.hwndFrom == hListSelectablePkgs)
			{
				sortColDir.first = pnmv->iSubItem;
				sortColDir.second = !sortColDir.second;
				SetSortHeader();
				SortSelectableItems();
			}
		}
		break;

	case NM_DBLCLK:
		{
			LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
			size_t iItem = static_cast<size_t>(lpnmitem->iItem);
			if (lpnmitem->hdr.hwndFrom == hListSelectablePkgs && iItem < selectableItems.size())
			{
				selectedItems.push_back(selectableItems[iItem]);
				FilterSelectableItems();
				InsertListViewItems(hListSelectedPkgs, selectedItems.size());
			}
			else if (lpnmitem->hdr.hwndFrom == hListSelectedPkgs && iItem < selectedItems.size())
			{
				selectedItems.erase(begin(selectedItems) + iItem);
				FilterSelectableItems();
				InsertListViewItems(hListSelectedPkgs, selectedItems.size());
			}
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONSTARTDEINSTALL), selectedItems.size() > 0);
		}
		break;
	}
	return Dialog::OnNotify(wParam, lParam);
}


INT_PTR DlgDeinstallSelectPkg::DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SHOWWINDOW:
		if (wParam)
		{
			FilterSelectableItems();
		}
		break;
	}
	return Dialog::DlgWndProc(message, wParam, lParam);
}

}
}
}