#include "stdafx.h"

#include <boost/algorithm/string.hpp>  

#include <Shellapi.h>
#include <strsafe.h>

#include <lhWinAPI.h>

#include "DBHelper.h"
#include "DlgFileDependencies.h"
#include "FileSystemUtils.h"
#include "TimeUtils.h"


namespace l3d
{
	namespace packageinstaller
	{
		namespace ui
		{

			DlgFileDependencies::DlgFileDependencies(std::wstring l3dPath) : dropTarget_{ {} }, l3dPath_(std::move(l3dPath)), curFile_{ -1 }, curDependency_{ -1 }
			{
			}

			DlgFileDependencies::~DlgFileDependencies()
			{
				//RevokeDragDrop();
			}

			void DlgFileDependencies::OnFilesDrop()
			{
				if (!dropTarget_.getFilenames().empty()) {
					auto p = fs::AbsoluteToLoksimRelativePath(dropTarget_.getFilenames()[0], l3dPath_);
					SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITFILTER), p.c_str());
					SearchFiles();
				}

			}

			HWND DlgFileDependencies::CreateModeless(HINSTANCE hInstance, HWND hWndParent)
			{
				return Dialog::CreateModeless(hInstance, MAKEINTRESOURCE(IDD_FILEDEPENDENCIES), hWndParent);
			}

			INT_PTR DlgFileDependencies::OnInitDlg(WPARAM wParam, LPARAM lParam)
			{
				hTxtFilter_ = GetDlgItem(GetHwnd(), IDC_EDITFILTER);

				hListFiles_ = GetDlgItem(GetHwnd(), IDC_LISTALLFILES);
				hListDependencies_ = GetDlgItem(GetHwnd(), IDC_LISTDEPENDENCIES);

				dropTarget_.SetOnDropNotify(this);

				RegisterDragDrop(hTxtFilter_, &dropTarget_);
				RegisterDragDrop(hListFiles_, &dropTarget_);

				SetUrlCtrl(GetDlgItem(GetHwnd(), IDC_BACKUPDIRECTORY));

				using l3d::packageinstaller::ui::resizeabledlghelper::ANCHOR;
				AddAnchor(ANCHOR(IDC_EDITFILTER, resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_TOP));
				AddAnchor(ANCHOR(IDC_LISTALLFILES, resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_TOP));
				AddAnchor(ANCHOR(IDC_LISTDEPENDENCIES, resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_TOP_AND_BOTTOM));


				AddAnchor(ANCHOR(IDC_STATICINSTALLAT, resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));
				AddAnchor(ANCHOR(IDC_EDITINSTALLAT, resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));

				AddAnchor(ANCHOR(IDC_STATICFILEEXISTED, resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));
				AddAnchor(ANCHOR(IDC_EDITFILEEXISTED, resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));

				AddAnchor(ANCHOR(IDC_STATICFILEOVERWRITTEN, resizeabledlghelper::AF_RIGHT | resizeabledlghelper::AF_BOTTOM));
				AddAnchor(ANCHOR(IDC_EDITFILEOVERWRITTEN, resizeabledlghelper::AF_RIGHT | resizeabledlghelper::AF_BOTTOM));

				AddAnchor(ANCHOR(IDC_BACKUPDIRECTORY, resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));
				AddAnchor(ANCHOR(IDC_STATICFILEPATH, resizeabledlghelper::AF_LEFT | resizeabledlghelper::AF_BOTTOM));
				AddAnchor(ANCHOR(IDC_EDITFILEPATH, resizeabledlghelper::AF_LEFT_AND_RIGHT | resizeabledlghelper::AF_BOTTOM));

				return ResizeableDialog::OnInitDlg(wParam, lParam);
			}

			/**
			* Hilfsfrozedur für den Info-Menüpunkt<br>
			* Anzeige HAnd beim Fahren über URLs
			*/
			static LRESULT CALLBACK UrlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
			{
				WNDPROC proc = (WNDPROC) GetWindowLongPtr(hWnd, GWLP_USERDATA);
				switch (uMsg) {
				case WM_SETCURSOR:
					SetCursor(LoadCursor(NULL, IDC_HAND));
					return(TRUE);
				}
				return (CallWindowProc(proc, hWnd, uMsg, wParam, lParam));
			}

			void DlgFileDependencies::SetUrlCtrl(HWND urlCtrl)
			{
				LOGFONT lf;
				HFONT hFont;

				if (nullptr == (hFont = (HFONT) SendMessage(urlCtrl, WM_GETFONT, 0, 0)))
					hFont = (HFONT) GetStockObject(SYSTEM_FONT);

				GetObject((HGDIOBJ) hFont, sizeof(lf), &lf);
				lf.lfUnderline = TRUE;
				hFont = CreateFontIndirect(&lf);

				SetWindowLongPtr(urlCtrl, GWLP_USERDATA,
								 SetWindowLongPtr(urlCtrl, GWLP_WNDPROC, (LONG_PTR) UrlProc));

				SendMessage(urlCtrl, WM_SETFONT, (WPARAM) hFont, (LPARAM) TRUE);

				fontsToDelete_.push_back(hFont);
			}


			INT_PTR DlgFileDependencies::DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam)
			{
				switch (message) {
				case WM_NCDESTROY:
					std::for_each(begin(fontsToDelete_), end(fontsToDelete_), [](HFONT f) { DeleteObject((HGDIOBJ) f); });
					break;

				case WM_CTLCOLORSTATIC:
					if (GetDlgItem(GetHwnd(), IDC_BACKUPDIRECTORY) == (HWND)lParam) {
						SetTextColor((HDC) wParam, GetSysColor(COLOR_HIGHLIGHT));
						SetBkMode((HDC) wParam, TRANSPARENT);
						return((BOOL) GetStockObject(NULL_BRUSH));
					}
					break;
				}
				return Dialog::DlgWndProc(message, wParam, lParam);
			}

			INT_PTR DlgFileDependencies::OnSize(WPARAM wParam, LPARAM lParam)
			{
				return ResizeableDialog::OnSize(wParam, lParam);
			}

			INT_PTR DlgFileDependencies::OnCommand(WPARAM wParam, LPARAM lParam)
			{
				if (HIWORD(wParam) == EN_CHANGE) {
					SearchFiles();
				}
				switch (LOWORD(wParam)) {
				case IDC_BACKUPDIRECTORY:
					if (curDependency_ >= 0 && static_cast<size_t>(curDependency_) < dependencies_.size()) {
						auto dir = fs::LoksimRelativeToAbsolutePath(
							db::DBHelper::instance().GetPackageBackupDir(dependencies_[curDependency_].first.GetPackageId()),
							l3dPath_);
						ShellExecute(GetHwnd(), L"explore", dir.c_str(), NULL, NULL, SW_SHOW);
					}
					return TRUE;
				}
				return Dialog::OnCommand(wParam, lParam);
			}

			INT_PTR DlgFileDependencies::OnNotify(WPARAM wParam, LPARAM lParam)
			{
				static HWND focusedListView = nullptr;

				std::wstring str;

				switch (((LPNMHDR) lParam)->code) {
				case LVN_GETDISPINFO:
				{
					NMLVDISPINFO* plvdi = (NMLVDISPINFO*) lParam;
					size_t iItem = static_cast<size_t>(plvdi->item.iItem);
					if (plvdi->hdr.hwndFrom == hListFiles_ && iItem < files_.size()) {
						StringCchCopy(plvdi->item.pszText,
									  plvdi->item.cchTextMax,
									  files_[iItem].GetFilepath().c_str());
					}
					else if (plvdi->hdr.hwndFrom == hListDependencies_ && iItem < dependencies_.size()) {
						const auto& d = dependencies_[iItem];
						if (d.first.WasInstalled()) {
							plvdi->item.pszText[0] = '*';
							StringCchCopy(plvdi->item.pszText + 1,
										  plvdi->item.cchTextMax - 1,
										  d.second.GetPackageName().c_str());
						}
						else {
							StringCchCopy(plvdi->item.pszText,
										  plvdi->item.cchTextMax,
										  d.second.GetPackageName().c_str());
						}
					}
					break;
				}

				case NM_SETFOCUS:
				{
					LPNMHDR lpnmh = (LPNMHDR) lParam;
					focusedListView = lpnmh->hwndFrom;
					break;
				}

				case LVN_ITEMCHANGED:
				case NM_CLICK:
				{
					int iItem = ListView_GetNextItem(focusedListView, -1, LVNI_SELECTED);
					if (iItem >= 0 && focusedListView == hListFiles_ && static_cast<size_t>(iItem) < files_.size() && files_.size() > 0) {
						if (curFile_ != iItem) {
							dependencies_ = fileSearch_.GetDependencies(files_[iItem].GetId());
							curDependency_ = -1;
							FillDependencyList();
							curFile_ = iItem;
						}
					}
					else if (iItem >= 0 && focusedListView == hListDependencies_ && static_cast<size_t>(iItem) < dependencies_.size()) {
						curDependency_ = iItem;
						const auto& d = dependencies_[iItem];
						auto str = FormatUnixTime(d.second.GetInstalledTimestamp());
						SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITINSTALLAT), str.c_str());
						SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITFILEPATH), d.second.GetPackagePath().c_str());
						SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITFILEOVERWRITTEN), d.first.WasInstalled() ?
									  strTable_.LoadS(IDS_YES).c_str() : strTable_.LoadS(IDS_NO).c_str());
						if (curFile_ >= 0 && static_cast<size_t>(curFile_) < files_.size()) {
							SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITFILEEXISTED), files_[curFile_].ExistedBefore() ?
										  strTable_.LoadS(IDS_YES).c_str() : strTable_.LoadS(IDS_NO).c_str());
						}
					}
					else {
						//SetWindowText(GetDlgItem(GetHwnd(), IDC_EDITREADME), L"");
					}
					break;
				}

				}
				return Dialog::OnNotify(wParam, lParam);
			}


			bool DlgFileDependencies::InsertListViewItems(HWND listView, size_t cntItems)
			{
				LVITEM lvI;

				ListView_DeleteAllItems(listView);

				// Initialize LVITEM members that are common to all items.
				lvI.pszText = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
				lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
				lvI.stateMask = 0;
				lvI.iSubItem = 0;
				lvI.state = 0;

				// Initialize LVITEM members that are different for each item.
				for (size_t index = 0; index < cntItems; index++) {
					lvI.iItem = index;
					lvI.iImage = index;

					// Insert items into the list.
					if (ListView_InsertItem(listView, &lvI) == -1) {
						return false;
					}
				}

				return true;
			}

			void DlgFileDependencies::FillDependencyList()
			{
				InsertListViewItems(hListDependencies_, dependencies_.size());
			}

			void DlgFileDependencies::SearchFiles()
			{
				std::wstring filterText = L"%";
				filterText.append(lhWinAPI::GetWindowString(GetDlgItem(GetHwnd(), IDC_EDITFILTER)));
				boost::replace_all(filterText, L"\\", L"/");
				filterText.append(L"%");

				boost::to_lower(filterText);
				if (lastSearch_ != filterText && filterText.length() > 4) {
					files_ = fileSearch_.SearchFilesByText(filterText);
					if (files_.size() == 1) {
						dependencies_ = fileSearch_.GetDependencies(files_[0].GetId());
						curFile_ = 0;
					}
					InsertListViewItems(hListFiles_, files_.size());
					FillDependencyList();
					lastSearch_ = filterText;
				}
			}
		}
	}
}
