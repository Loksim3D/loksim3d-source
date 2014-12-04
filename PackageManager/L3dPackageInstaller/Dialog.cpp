#include "StdAfx.h"
#include "Dialog.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

Dialog::Dialog(void)
{
	minSize.x = -1;
	minSize.y = -1;
}


Dialog::~Dialog(void)
{
}

INT_PTR Dialog::Show(	__in_opt HINSTANCE hInstance,
					__in LPCWSTR lpTemplateName,
					__in_opt HWND hWndParent)
{
	this->hInstance = hInstance;
	return DialogBoxParam(hInstance, lpTemplateName, hWndParent, StaticWndProc, reinterpret_cast<LPARAM>(this));
}

HWND Dialog::CreateModeless(__in_opt  HINSTANCE hInstance,
							__in      LPCTSTR lpTemplateName,
							__in_opt  HWND hWndParent)
{
	this->hInstance = hInstance;
	hDlg = CreateDialogParam(hInstance, lpTemplateName, hWndParent, StaticWndProc, reinterpret_cast<LPARAM>(this));
	return hDlg;
}


INT_PTR Dialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	//if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
	//{
	//	EndDialog(hDlg, LOWORD(wParam));
	//}
	return FALSE;
}

INT_PTR Dialog::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

INT_PTR Dialog::OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam)
{
	MINMAXINFO *inf = reinterpret_cast<MINMAXINFO*>(lParam);
	if (inf != nullptr &&  (minSize.x != -1 || minSize.y != -1))
	{
		if (minSize.x != -1)
		{
			inf->ptMinTrackSize.x = minSize.x;
		}
		if (minSize.y != -1)
		{
			inf->ptMinTrackSize.y = minSize.y;
		}
		return 0;
	}
	return 1;
}

INT_PTR CALLBACK Dialog::StaticWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	Dialog* ptr = reinterpret_cast<Dialog*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));
	switch (message)
	{
	case WM_INITDIALOG:
		ptr = reinterpret_cast<Dialog*>(lParam);
		ptr->hDlg = hDlg;
		SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
		return ptr->DlgWndProc(message, wParam, lParam);

	default:
		if (ptr != nullptr)
		{
			return ptr->DlgWndProc(message, wParam, lParam);
		}
	}
	return (INT_PTR)FALSE;
}

INT_PTR Dialog::DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return OnInitDlg(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_NOTIFY:
		return OnNotify(wParam, lParam);
	case WM_TIMER:
		return OnTimer(wParam, lParam);
	case WM_CLOSE:
		return OnClose(wParam, lParam);
	case WM_DESTROY:
		return OnDestroy(wParam, lParam);
	case WM_SIZE:
		return OnSize(wParam, lParam);
	case WM_GETMINMAXINFO:
		return OnGetMinMaxInfo(wParam, lParam);
	}
	return (INT_PTR)FALSE;
}

}
}
}