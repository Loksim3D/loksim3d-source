#include "StdAfx.h"

#include <Commctrl.h>

#include <lhWinAPI.h>

#include "DBHelper.h"
#include "DlgDbUpdate.h"


namespace l3d
{
	namespace packageinstaller
	{
		namespace ui
		{

			namespace
			{
				const UINT WM_FINISHED_UPDATE = WM_USER + 1;

				void DoDbUpdateAndPostAfterFinish(const std::wstring& l3dPath, HWND hwndDlg)
				{
					db::DBHelper::instance().UpdateDb(l3dPath);
					PostMessage(hwndDlg, WM_FINISHED_UPDATE, 0, 0);
				}
			}


			DlgDbUpdate::DlgDbUpdate(std::wstring l3dPath) : l3dPath_(std::move(l3dPath))
			{
			}

			INT_PTR DlgDbUpdate::Show(HINSTANCE hInstance, HWND hWndParent)
			{
				return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_DBUPDATE), hWndParent);
			}

			INT_PTR DlgDbUpdate::OnCommand(WPARAM wParam, LPARAM lParam)
			{
				return FALSE;
			}

			INT_PTR DlgDbUpdate::OnInitDlg(WPARAM wParam, LPARAM lParam)
			{
				HWND hwndProgress = GetDlgItem(GetHwnd(), IDC_PROGRESS);
				DWORD dwStyle = ::GetWindowLong(hwndProgress, GWL_STYLE);
				::SetWindowLong(hwndProgress, GWL_STYLE, dwStyle | PBS_MARQUEE);
				::SendMessage(hwndProgress, (UINT) PBM_SETMARQUEE, (WPARAM) TRUE, (LPARAM) 0);
				auto r = Dialog::OnInitDlg(wParam, lParam);
				updateTask_ = std::async(std::launch::async,
										 DoDbUpdateAndPostAfterFinish, l3dPath_, GetHwnd());
				return r;
			}

			INT_PTR DlgDbUpdate::DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam)
			{
				if (message == WM_FINISHED_UPDATE) {
					EndDialog(GetHwnd(), 0);
				}
				return Dialog::DlgWndProc(message, wParam, lParam);
			}

		}
	}
}
