#include "StdAfx.h"
#include "L3dPackageInstaller.h"
#include "DlgInstallSelectFiles.h"
#include "TimeUtils.h"

#include "DBHelper.h"

#include <Shellapi.h>
#include <windowsx.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <lhWinAPI.h>

#include <time.h>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{


using namespace std;
using namespace insthelper;


DlgInstallSelectFiles::DlgInstallSelectFiles(const std::map<std::wstring, std::unique_ptr<InstallInformation>>* fileMap, 
	std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> deinstalledFiles, bool userShouldSelect) : fileMap(fileMap), userShouldSelect(userShouldSelect),
	selectedFiles(make_shared<map<std::wstring, UserSelection>>()), onlyDisplayUserSelection(false), deinstalledFiles(deinstalledFiles)
{
}

DlgInstallSelectFiles::DlgInstallSelectFiles(const std::map<std::wstring, std::unique_ptr<insthelper::InstallInformation>>* fileMap, 
	std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> deinstalledFiles, bool userShouldSelect,
	std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> selFiles) : fileMap(fileMap), userShouldSelect(userShouldSelect),
	selectedFiles(selFiles), onlyDisplayUserSelection(!userShouldSelect), deinstalledFiles(deinstalledFiles)
{
}

DlgInstallSelectFiles::~DlgInstallSelectFiles(void)
{
}

INT_PTR DlgInstallSelectFiles::Show(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_INSTALLSELECTFILES), hWndParent);
}

INT_PTR DlgInstallSelectFiles::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	if (userShouldSelect)
	{
		hwndTree = GetDlgItem(GetHwnd(), IDC_TREEFILESCHECKABLE);
		SetWindowText(GetHwnd(), strTable.Load(IDS_HEADERINSTALLSELECTFILES));
	}
	else
	{
		hwndTree = GetDlgItem(GetHwnd(), IDC_TREEFILESNOTCHECKABLE);
		SetWindowText(GetHwnd(), strTable.Load(IDS_HEADERINSTALLSHOWFILES));
	}
	ShowWindow(GetDlgItem(GetHwnd(), IDC_TREEFILESCHECKABLE), userShouldSelect ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(GetHwnd(), IDC_STATICSELECTINSTALLFILES), userShouldSelect ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(GetHwnd(), IDCANCEL), userShouldSelect ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(GetHwnd(), IDC_TREEFILESNOTCHECKABLE), userShouldSelect ? SW_HIDE : SW_SHOW);
	ShowWindow(GetDlgItem(GetHwnd(), IDC_STATICSHOWINSTALLFILES), userShouldSelect ? SW_HIDE : SW_SHOW);

	HTREEITEM h;
	if (fileMap != nullptr)
	{
		for (auto en = fileMap->begin(); en != fileMap->end(); ++en)
		{
			boost::filesystem::path p(en->first);
			h = lhWinAPI::AddItemToTree(hwndTree, p.filename().c_str(), 1, 0, 0);
			TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
	
			if (!en->second->writeProtectedFilesToInstall.empty())
			{
				h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(userShouldSelect ? IDS_SELECTFILESTVWRITEPROTINSTALL : IDS_SHOWFILESTVWRITEPROTINSTALLED), 2, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				auto& userSel = (*selectedFiles)[en->first].filesToInstall;
				for (auto f = en->second->writeProtectedFilesToInstall.begin(); f != en->second->writeProtectedFilesToInstall.end(); ++f)
				{
					if (!onlyDisplayUserSelection || userSel.find(f->fileName) != userSel.end())
					{
						vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
						h = lhWinAPI::AddItemToTree(hwndTree, f->fileName.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
						TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
					}
				}
			}

			if (!en->second->writeProtectedFilesToDelete.empty())
			{
				h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(userShouldSelect ? IDS_SELECTFILESTVWRITEPROTDELETE : IDS_SHOWFILESTVWRITEPROTDELETED), 2, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				auto& userSel = (*selectedFiles)[en->first].filesToDelete;
				for (auto f = en->second->writeProtectedFilesToDelete.begin(); f != en->second->writeProtectedFilesToDelete.end(); ++f)
				{
					if (!onlyDisplayUserSelection || userSel.find(f->fileName) != userSel.end())
					{
						vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
						h = lhWinAPI::AddItemToTree(hwndTree, f->fileName.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
						TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
					}
				}
			}

			if (!en->second->filesToInstall.empty())
			{
				h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(userShouldSelect ? IDS_SELECTFILESTVINSTALLEXISTING : IDS_SHOWFILESTVINSTALLEDEXISTING), 2, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				auto& userSel = (*selectedFiles)[en->first].filesToInstall;
				for (auto f = en->second->filesToInstall.begin(); f != en->second->filesToInstall.end(); ++f)
				{
					if (f->FileExistsOnDisk())
					{
						if (!onlyDisplayUserSelection || userSel.find(f->fileName) != userSel.end())
						{
							vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
							h = lhWinAPI::AddItemToTree(hwndTree, f->fileName.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
							TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
						}
					}
				}

				h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(userShouldSelect ? IDS_SELECTFILESTVINSTALLNEW : IDS_SHOWFILESTVINSTALLEDNEW), 2, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				for (auto f = en->second->filesToInstall.begin(); f != en->second->filesToInstall.end(); ++f)
				{
					if (!f->FileExistsOnDisk())
					{
						if (!onlyDisplayUserSelection || userSel.find(f->fileName) != userSel.end())
						{
							vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
							h = lhWinAPI::AddItemToTree(hwndTree, f->fileName.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
							TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
						}
					}
				}
			}

			if (!en->second->filesToDelete.empty())
			{
				h = lhWinAPI::AddItemToTree(hwndTree, strTable.Load(userShouldSelect ? IDS_SELECTFILESTVDELETE : IDS_SHOWFILESTVDELETED), 2, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				auto& userSel = (*selectedFiles)[en->first].filesToDelete;
				for (auto f = en->second->filesToDelete.begin(); f != en->second->filesToDelete.end(); ++f)
				{
					if (!onlyDisplayUserSelection || userSel.find(f->fileName) != userSel.end())
					{
						vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(en->first, *f)));
						h = lhWinAPI::AddItemToTree(hwndTree, f->fileName.c_str(), 3, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
						TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
					}
				}
			}
		}

		if (deinstalledFiles)
		{
			for (auto p = deinstalledFiles->cbegin(); p != deinstalledFiles->cend(); ++p)
			{
				wstring deinstallHeader = strTable.LoadS(IDS_DEINSTALLED_PKG_HEADER);
				boost::wformat fmt(deinstallHeader);
				h = lhWinAPI::AddItemToTree(hwndTree, (fmt % p->first.GetPackageName().c_str()).str(), 1, 0, 0);
				TreeView_SetItemState (hwndTree, h, TVIS_EXPANDED, TVIS_EXPANDED);
				for (auto f = p->second.cbegin(); f != p->second.cend(); ++f)
				{
					vecTvItemData.emplace_back(unique_ptr<TvItemData>(new TvItemData(p->first.GetPackageName(), FileInstallInfo(*f), p->first.GetId())));
					h = lhWinAPI::AddItemToTree(hwndTree, f->c_str(), 2, reinterpret_cast<LPARAM>(vecTvItemData.back().get()), 0);
					if (userShouldSelect)
					{
						// Items "Ausgrauen"
						TVITEMEX stateEx = { 0 };
						stateEx.mask = TVIF_STATEEX;
						stateEx.hItem = h;
						stateEx.uStateEx = TVIS_EX_DISABLED;
						TreeView_SetItem(hwndTree, &stateEx);
					}
				}
			}
		}
	}

	if (userShouldSelect)
	{
		SetTimer(GetHwnd(), 0, 50, nullptr);
	}
	return Dialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgInstallSelectFiles::OnTimer(WPARAM wParam, LPARAM lParam)
{
	ForAllTvItems([this] (const HTREEITEM& item) {
		TreeView_SetCheckState(hwndTree, item, /*!TreeView_GetCheckState(hwndTree, item)*/ 1);
	});

	KillTimer(GetHwnd(), 0);
	return Dialog::OnTimer(wParam, lParam);
}

void DlgInstallSelectFiles::SetCheckStateAllChilds(HTREEITEM parent, BOOL fCheck)
{
	ForAllTvChildItems(parent, [this, &fCheck] (const HTREEITEM& item) {
		TreeView_SetCheckState(hwndTree, item, fCheck);
	});
}

INT_PTR DlgInstallSelectFiles::OnNotify(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDC_TREEFILESCHECKABLE || LOWORD(wParam) == IDC_TREEFILESNOTCHECKABLE)
	{
		LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
		TVHITTESTINFO ht = {0};
		if(lpnmh->code  == NM_CLICK && lpnmh->hwndFrom == hwndTree)
		{
			DWORD dwpos = GetMessagePos();

			ht.pt.x = GET_X_LPARAM(dwpos);
			ht.pt.y = GET_Y_LPARAM(dwpos);
			MapWindowPoints(HWND_DESKTOP, lpnmh->hwndFrom, &ht.pt, 1);

			TreeView_HitTest(lpnmh->hwndFrom, &ht);
		 
			TVITEM tvitem = { 0 };
			tvitem.hItem = ht.hItem;
			TreeView_GetItem(hwndTree, &tvitem);
			TvItemData *d = reinterpret_cast<TvItemData*>(tvitem.lParam);
			UINT state = TreeView_GetCheckState(hwndTree, ht.hItem);
			if (d != nullptr && d->fileInfo.IsFileToDeinstall())
			{
				TreeView_SetCheckState(hwndTree, ht.hItem, !state);
			}
			else if(TVHT_ONITEMSTATEICON & ht.flags)
			{
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
				if (d->fileInfo.FileExistsOnDisk())
				{
					SetDlgItemText(GetHwnd(), IDC_INSTSELECTFSMTIME, FormatUnixTime(d->fileInfo.modifiedTimeFileSystem).c_str());
				}
				if (d->fileInfo.IsFileToInstall())
				{
					SetDlgItemText(GetHwnd(), IDC_INSTSELECTPKGMTIME, FormatUnixTime(d->fileInfo.modifiedTimePackage).c_str());
				}
			}
		}
		else if (lpnmh->code == NM_DBLCLK && lpnmh->hwndFrom == hwndTree && !userShouldSelect)
		{
			HTREEITEM curSel = TreeView_GetSelection(hwndTree);
			if (curSel != nullptr)
			{
				TVITEM tvitem = { 0 };
				tvitem.hItem = curSel;
				TreeView_GetItem(hwndTree, &tvitem);
				TvItemData *d = reinterpret_cast<TvItemData*>(tvitem.lParam);
				if (d != nullptr)
				{
					boost::filesystem::path fp = l3dPath / d->fileInfo.fileName;
					if (boost::filesystem::exists(fp))
					{
						OutputDebugString(fp.c_str());
						ShellExecute(GetHwnd(), L"open", fp.c_str(), nullptr, nullptr, SW_NORMAL);
					}
				}
			}

		}
	}
	return Dialog::OnNotify(wParam, lParam);
}

INT_PTR DlgInstallSelectFiles::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (LOWORD(wParam) == IDOK)
	{
		if (userShouldSelect)
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
						if (d->fileInfo.IsFileToDelete())
						{
							(*selectedFiles)[d->packageName].filesToDelete.insert(d->fileInfo.fileName);
						}
						else if (d->fileInfo.IsFileToDeinstall())
						{
							(*selectedFiles)[d->packageName].packagesToDeinstall.insert(d->pkgId);
						}
						else
						{
							(*selectedFiles)[d->packageName].filesToInstall.insert(d->fileInfo.fileName);
						}
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