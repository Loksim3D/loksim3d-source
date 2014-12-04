#include "StdAfx.h"
#include "L3dPackageInstaller.h"
#include "DlgDeInstallSelectFiles.h"
#include "TimeUtils.h"

#include <windowsx.h>

#include <boost/filesystem.hpp>
#include <lhWinAPI.h>

#include "DBHelper.h"
#include <time.h>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

using namespace std;
using namespace db;

DlgDeInstallSelectFiles::DlgDeInstallSelectFiles(std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> filesToDeinstall, std::shared_ptr<std::map<int, std::wstring>> pkgIdToNameMap) : 
	filesToDeinstall(filesToDeinstall), pkgIdToNameMap(pkgIdToNameMap), selectedFiles(make_shared<std::map<int, std::vector<std::wstring>>>())
{
}

DlgDeInstallSelectFiles::DlgDeInstallSelectFiles(std::shared_ptr<std::map<int, std::vector<std::wstring>>> deinstalledFiles, std::shared_ptr<std::map<int, std::wstring>> pkgIdToNameMap) : 
	deinstalledFiles(deinstalledFiles), pkgIdToNameMap(pkgIdToNameMap),  selectedFiles(make_shared<std::map<int, std::vector<std::wstring>>>())
{
}

DlgDeInstallSelectFiles::~DlgDeInstallSelectFiles(void)
{
}

INT_PTR DlgDeInstallSelectFiles::Show(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_DEINSTALLSELECTFILES), hWndParent);
}

INT_PTR DlgDeInstallSelectFiles::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	if (filesToDeinstall)
	{
		hwndTree = GetDlgItem(GetHwnd(), IDC_TREEFILESCHECKABLE);
		SetWindowText(GetHwnd(), strTable.Load(IDS_HEADERDEINSTALLSELECTFILES));
		SetDlgItemText(GetHwnd(), IDC_STATICDEINSTALLFILESDLGTXT, strTable.Load(IDS_DEINSTALLSELECTFILES));
	}
	else
	{
		hwndTree = GetDlgItem(GetHwnd(), IDC_TREEFILESNOTCHECKABLE);
		SetWindowText(GetHwnd(), strTable.Load(IDS_HEADERDEINSTALLSHOWFILES));
		SetDlgItemText(GetHwnd(), IDC_STATICDEINSTALLFILESDLGTXT, strTable.Load(IDS_DEINSTALLSELECTFILESSHOW));
		ShowWindow(GetDlgItem(GetHwnd(), IDC_STATICDEINSTALLFSMT), SW_HIDE);
		ShowWindow(GetDlgItem(GetHwnd(), IDC_DEINSTSTALLFSMTIME), SW_HIDE);
	}
	ShowWindow(GetDlgItem(GetHwnd(), IDC_TREEFILESCHECKABLE), filesToDeinstall ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(GetHwnd(), IDCANCEL), filesToDeinstall ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(GetHwnd(), IDC_TREEFILESNOTCHECKABLE), filesToDeinstall ? SW_HIDE : SW_SHOW);

	HTREEITEM h;
	if (filesToDeinstall)
	{
		for (auto en = filesToDeinstall->begin(); en != filesToDeinstall->end(); ++en)
		{
			h = lhWinAPI::AddItemToTree(hwndTree, (*pkgIdToNameMap)[en->first].c_str(), 1, 0, 0);
			TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
			
			if (!en->second.empty())
			{
				if (find_if(begin(en->second), end(en->second), [](const FileToDeinstall& deEn) { return deEn.writeProtected; }) != end(en->second))
				{
					h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(IDS_SELECTFILESTVWRITEPROTDELETE), 2, 0, 0);
					TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
					for (auto f = begin(en->second); f != end(en->second); ++f)
					{
						if (f->writeProtected)
						{
							vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
							h = lhWinAPI::AddItemToTree(hwndTree, f->filename.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
							TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
						}
					}
				}

				h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(IDS_SELECTFILESTVDELETE), 2, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				for (auto f = begin(en->second); f != end(en->second); ++f)
				{
					if (!f->writeProtected)
					{
						vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
						h = lhWinAPI::AddItemToTree(hwndTree, f->filename.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
						TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
					}
				}
			}
		}
		SetTimer(GetHwnd(), 0, 50, nullptr);
	}
	else if (deinstalledFiles)
	{
		for (auto en = deinstalledFiles->begin(); en != deinstalledFiles->end(); ++en)
		{
			h = lhWinAPI::AddItemToTree(hwndTree, (*pkgIdToNameMap)[en->first].c_str(), 1, 0, 0);
			TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
			
			for (auto f = begin(en->second); f != end(en->second); ++f)
			{
				h = lhWinAPI::AddItemToTree(hwndTree, f->c_str(), 2, 0, 0);
			}
		}
	}
	return Dialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgDeInstallSelectFiles::OnTimer(WPARAM wParam, LPARAM lParam)
{
	ForAllTvItems([this] (const HTREEITEM& item) {
		TreeView_SetCheckState(hwndTree, item, /*!TreeView_GetCheckState(hwndTree, item)*/ 1);
	});

	KillTimer(GetHwnd(), 0);
	return Dialog::OnTimer(wParam, lParam);
}

void DlgDeInstallSelectFiles::SetCheckStateAllChilds(HTREEITEM parent, BOOL fCheck)
{
	ForAllTvChildItems(parent, [this, &fCheck] (const HTREEITEM& item) {
		TreeView_SetCheckState(hwndTree, item, fCheck);
	});
}

INT_PTR DlgDeInstallSelectFiles::OnNotify(WPARAM wParam, LPARAM lParam)
{
	LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
	TVHITTESTINFO ht = {0};

	if (filesToDeinstall)
	{
		if(lpnmh->code  == NM_CLICK && lpnmh->hwndFrom == hwndTree)
		{
			DWORD dwpos = GetMessagePos();

			ht.pt.x = GET_X_LPARAM(dwpos);
			ht.pt.y = GET_Y_LPARAM(dwpos);
			MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);

			TreeView_HitTest(lpnmh->hwndFrom, &ht);
		 
			if(TVHT_ONITEMSTATEICON & ht.flags)
			{
				UINT state = TreeView_GetCheckState(hwndTree, ht.hItem);
				SetCheckStateAllChilds(ht.hItem, !state);
			}
		} 
		else if (lpnmh->code == TVN_SELCHANGED && lpnmh->hwndFrom == hwndTree)
		{
			NMTREEVIEW *tv = reinterpret_cast<NMTREEVIEW*>(lParam);
			TvItemData *d = reinterpret_cast<TvItemData*>(tv->itemNew.lParam);
			SetDlgItemText(GetHwnd(), IDC_INSTSELECTFSMTIME, L"");
			SetDlgItemText(GetHwnd(), IDC_INSTSELECTPKGMTIME, L"");
			if (d != nullptr)
			{
				SetDlgItemText(GetHwnd(), IDC_DEINSTSTALLFSMTIME,  FormatUnixTime(d->fileInfo.modifiedTimestamp).c_str());
			}
		}
	}
	return Dialog::OnNotify(wParam, lParam);
}

INT_PTR DlgDeInstallSelectFiles::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDOK)
	{
		if (filesToDeinstall)
		{
			TVITEM tvitem = { 0 };
			tvitem.mask = TVIF_PARAM;
			ForAllTvItems([this, &tvitem] (const HTREEITEM& item) {	
				if (TreeView_GetCheckState(hwndTree, item) == 1)
				{
					tvitem.hItem = item;
					TreeView_GetItem(hwndTree, &tvitem);
					TvItemData *d = reinterpret_cast<TvItemData*>(tvitem.lParam);
					if (d != nullptr)
					{
						(*selectedFiles)[d->pkgId].push_back(d->fileInfo.filename);
					}
				}
			});
		}
		EndDialog(GetHwnd(), LOWORD(wParam));
	}
	else if (LOWORD(wParam) == IDCANCEL)
	{
		EndDialog(GetHwnd(), LOWORD(wParam));
	}
	return Dialog::OnCommand(wParam, lParam);
}

}
}
}