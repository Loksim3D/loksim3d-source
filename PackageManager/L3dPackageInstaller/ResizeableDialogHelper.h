#pragma once

#include "stdafx.h"
#include "ResizeableDialogHelperTypes.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{
namespace resizeabledlghelper
{

// to enable a dialog to be resizable do the following after including this header:
//
// 1) in your class declare
//      private:
//          static const ANCHOR c_rgAnchors[5];
//          RECT _rgAnchorOffsets[ARRAYSIZE(c_rgAnchors)];
//
// 2) declare an entry in this table for every control in your dialog, for example
//
//      const ANCHOR CShellQueryInspectorApp::c_rgAnchors[] =
//      {
//          { IDC_QUERY,        AF_LEFT | AF_BOTTOM },
//          { IDC_OPEN_ITEM,    AF_LEFT | AF_BOTTOM },
//          { IDC_PICK,         AF_LEFT | AF_BOTTOM },
//          { IDC_STATIC,       AF_LEFT | AF_RIGHT | AF_TOP | AF_BOTTOM },
//          { IDC_LISTVIEW,     AF_LEFT | AF_RIGHT | AF_TOP | AF_BOTTOM },
//      };
//
// 3) in your DialogProc add
//      case WM_SIZE:
//          OnSize(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);
//          break;
//
//      case WM_INITDIALOG:
//          InitResizeData(_hdlg, c_rgAnchors, ARRAYSIZE(c_rgAnchors), _rgAnchorOffsets);
//          break;
//



inline void GetWindowRectInClient(HWND hwnd, RECT *prc)
{
	GetWindowRect(hwnd, prc);
	MapWindowPoints(GetDesktopWindow(), GetParent(hwnd), (POINT*)prc, 2);
}


template<class AnchorForwardIterator, class RectForwardIterator>
void InitResizeData(HWND hdlg, AnchorForwardIterator firstAnchor, AnchorForwardIterator lastAnchor, RectForwardIterator firstAnchorOffset)
{
	// record anchor information
	RECT rcClient = {};
	GetClientRect(hdlg, &rcClient);
	
	for (; firstAnchor != lastAnchor; ++firstAnchor, ++firstAnchorOffset)
	{
		const ANCHOR& curAnchor = *firstAnchor;
		RECT& curAnchorOffset = *firstAnchorOffset;
		GetWindowRectInClient(GetDlgItem(hdlg, curAnchor.idControl), &curAnchorOffset);

		switch (curAnchor.aff & AF_LEFT_AND_RIGHT)
		{
		case AF_LEFT_AND_RIGHT:
			curAnchorOffset.left -= rcClient.left;    // offset
			curAnchorOffset.right -= rcClient.right;  // offset
			break;

		case AF_RIGHT:
			curAnchorOffset.left = curAnchorOffset.right - curAnchorOffset.left;  // width
			curAnchorOffset.right -= rcClient.right;  // offset
			break;
		}

		switch (curAnchor.aff & AF_TOP_AND_BOTTOM)
		{
		case AF_TOP_AND_BOTTOM:
			curAnchorOffset.top -= rcClient.top;       // offset
			curAnchorOffset.bottom -= rcClient.bottom; // offset
			break;

		case AF_BOTTOM:
			curAnchorOffset.top = curAnchorOffset.bottom - curAnchorOffset.top;   // height
			curAnchorOffset.bottom -= rcClient.bottom; // offset
			break;
		}
	}
}

template<class AnchorBidirectionalIterator, class RectForwardIterator>
void OnSize(HWND hdlg, AnchorBidirectionalIterator firstAnchor, AnchorBidirectionalIterator lastAnchor, RectForwardIterator firstAnchorOffset)
{
	size_t cAnchors = 0;
	for (auto it = firstAnchor; it != lastAnchor; ++it)
	{
		++cAnchors;
	}
	HDWP hdwp = BeginDeferWindowPos(cAnchors);
	if (hdwp)
	{
		RECT rcClient;
		GetClientRect(hdlg, &rcClient);

		for (; firstAnchor != lastAnchor; ++firstAnchor, ++firstAnchorOffset)
		{
			const ANCHOR& curAnchor = *firstAnchor;
			const RECT& curAnchorOffset = *firstAnchorOffset;

			const HWND hwndControl = GetDlgItem(hdlg, curAnchor.idControl);
			RECT rcNewPos;
			GetWindowRectInClient(hwndControl, &rcNewPos);

			switch (curAnchor.aff & AF_LEFT_AND_RIGHT)
			{
			case AF_RIGHT:
				rcNewPos.right = rcClient.right + curAnchorOffset.right;
				rcNewPos.left = rcNewPos.right - curAnchorOffset.left; // rgAnchorOffsets[iAnchor].left contains the width of the control
				break;
			case AF_LEFT_AND_RIGHT:
				rcNewPos.right = rcClient.right + curAnchorOffset.right;
				break;
			}

			switch (curAnchor.aff & AF_TOP_AND_BOTTOM)
			{
			case AF_BOTTOM:
				rcNewPos.bottom = rcClient.bottom + curAnchorOffset.bottom;
				rcNewPos.top = rcNewPos.bottom - curAnchorOffset.top; // rgAnchorOffsets[iAnchor].top contains the height of the control
				break;
			case AF_TOP_AND_BOTTOM:
				rcNewPos.bottom = rcClient.bottom + curAnchorOffset.bottom;
				break;
			}

			MoveWindow(hwndControl, rcNewPos.left, rcNewPos.top, rcNewPos.right - rcNewPos.left,  rcNewPos.bottom - rcNewPos.top, TRUE);

			DeferWindowPos(hdwp, hwndControl, 0, rcNewPos.left, rcNewPos.top, rcNewPos.right - rcNewPos.left,  rcNewPos.bottom - rcNewPos.top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		EndDeferWindowPos(hdwp);
	}
}

// Set up common controls v6 the easy way.  By doing this, there is no need
// to call InitCommonControlsEx().
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

}
}
}
}