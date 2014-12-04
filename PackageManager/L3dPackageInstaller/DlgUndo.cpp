#include "StdAfx.h"
#include "DlgUndo.h"

#include "DBHelper.h"
#include "TimeUtils.h"
#include <strsafe.h>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

using namespace std;
using namespace db;

DlgUndo::DlgUndo(const boost::filesystem::path& l3dPath) : l3dPath(l3dPath)
{
}


DlgUndo::~DlgUndo(void)
{
	if (deinstallMgmt)
	{
		deinstallMgmt->CancelAndWaitForTasks();
	}
}

HWND DlgUndo::CreateModeless(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::CreateModeless(hInstance, MAKEINTRESOURCE(IDD_UNDOOPERATION), hWndParent);
}

INT_PTR DlgUndo::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	hListSelectablePkgs = GetDlgItem(GetHwnd(), IDC_LISTUNDOPACKAGES);
	InitListViewColumns();

	//InsertListViewItems(hListSelectablePkgs, selectableItems.size());

	SetTimer(GetHwnd(), 0, 500, nullptr);

	
	using l3d::packageinstaller::ui::resizeabledlghelper::ANCHOR;
	AddAnchor(ANCHOR(IDC_LISTUNDOPACKAGES,	resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_TOP_AND_BOTTOM));
	AddAnchor(ANCHOR(IDC_BUTTONCANCELUNDO,	resizeabledlghelper::AF_RIGHT | resizeabledlghelper::AF_BOTTOM));
	AddAnchor(ANCHOR(IDC_BUTTONUNDO,		resizeabledlghelper::AF_RIGHT | resizeabledlghelper::AF_TOP));

	return ResizeableDialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgUndo::OnSize(WPARAM wParam, LPARAM lParam)
{
	return ResizeableDialog::OnSize(wParam, lParam);
}

bool DlgUndo::InitListViewColumns() 
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
		switch (iCol)
		{
		case 0:
			lvc.pszText = strTable.Load(IDS_PACKAGENAME);
			lvc.cx = 250;
			lvc.cxDefault = 250;
			break;
		case 1:
			lvc.pszText = strTable.Load(IDS_UNDODEORINSTALLEDON);
			lvc.cx = 120;
			lvc.cxDefault = 120;
			break;
		}

		lvc.fmt = LVCFMT_LEFT;  // Left-aligned column.

		// Insert the columns into the list view.
		if (ListView_InsertColumn(hListSelectablePkgs, iCol, &lvc) == -1)
			return false;
	}
	
	return true;
}

bool DlgUndo::InsertListViewItems(HWND listView, size_t cntItems)
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

bool DlgUndo::IsOperationRunning() const
{
	if (deinstallMgmt)
	{
		return !deinstallMgmt->IsFinished();
	}
	return false;
}

bool DlgUndo::SoftCancel()
{
	if (deinstallMgmt)
	{
		deinstallMgmt->CancelDeinstall();
	}
	return true;
}

void DlgUndo::SearchSelectableItems()
{
	displayedItems = DBHelper::instance().GetInstalledPackages();
	InsertListViewItems(hListSelectablePkgs, displayedItems.size());
}

INT_PTR DlgUndo::DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SHOWWINDOW:
		if (wParam)
		{
			SearchSelectableItems();
		}
		break;
	}
	return Dialog::DlgWndProc(message, wParam, lParam);
}

INT_PTR DlgUndo::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDC_BUTTONUNDO:
		{
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONUNDO), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELUNDO), TRUE);
			DBUnOrInstallPkgInfo& inf = displayedItems[0];
			if (inf.IsInstalledPkg())
			{
				std::vector<int> v;
				v.push_back(inf.GetId());
				deinstallMgmt.reset(new DeinstallManager(l3dPath, std::move(v)));
				deinstallMgmt->SetUndoInstallation(true);
				deinstallMgmt->StartDeinstall();
			}
		}
		break;

	case IDC_BUTTONCANCELUNDO:
		SoftCancel();
		break;
	}
	return Dialog::OnCommand(wParam, lParam);
}

INT_PTR DlgUndo::OnTimer(WPARAM wParam, LPARAM lParam)
{
	if (deinstallMgmt)
	{
		if (deinstallMgmt->IsFinished())
		{
			//EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONUNDO), TRUE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELUNDO), FALSE);
			deinstallMgmt.reset(nullptr);
			SearchSelectableItems();
		}
	}
	return Dialog::OnTimer(wParam, lParam);
}

INT_PTR DlgUndo::OnNotify(WPARAM wParam, LPARAM lParam)
{
	std::wstring str;

	switch (((LPNMHDR) lParam)->code)
	{
	case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = (NMLVDISPINFO*)lParam;	
			size_t iItem = static_cast<size_t>(plvdi->item.iItem);
			if (plvdi->hdr.hwndFrom == hListSelectablePkgs && iItem < displayedItems.size())
			{
				switch (plvdi->item.iSubItem)
				{
				case 0:
					StringCchCopy(	plvdi->item.pszText, 
									plvdi->item.cchTextMax,
									displayedItems[iItem].GetPackageName().c_str());
					break;
					  
				case 1:
					str = FormatUnixTime(displayedItems[iItem].GetInstalledTimestamp());
					StringCchCopy(	plvdi->item.pszText, 
									plvdi->item.cchTextMax,
									str.c_str());
					break;                               
				default:
					break;
				}
			}
		}
		break;
	case LVN_ITEMCHANGED:
	case NM_CLICK:
		{
			int iItem = ListView_GetNextItem(hListSelectablePkgs, -1, LVNI_SELECTED);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONUNDO), iItem == 0 && !deinstallMgmt);
		}
		break;
	}
	return Dialog::OnNotify(wParam, lParam);
}

}
}
}