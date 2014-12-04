#include "StdAfx.h"
#include "ResizeableDialog.h"
#include "ResizeableDialogHelper.h"
namespace l3d
{
namespace packageinstaller
{
namespace ui
{


ResizeableDialog::ResizeableDialog(void) : _initialized(false)
{
}


ResizeableDialog::~ResizeableDialog(void)
{
}

INT_PTR ResizeableDialog::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	resizeabledlghelper::InitResizeData(GetHwnd(), begin(_anchors), end(_anchors), begin(_anchorOffsets));
	_initialized = true;
	return Dialog::OnInitDlg(wParam, lParam);
}

INT_PTR ResizeableDialog::OnSize(WPARAM wParam, LPARAM lParam)
{
	if (_initialized)
	{
		resizeabledlghelper::OnSize(GetHwnd(), begin(_anchors), end(_anchors), begin(_anchorOffsets));
		InvalidateRect(GetHwnd(), nullptr, FALSE);
	}
	return Dialog::OnSize(wParam, lParam);
}

void ResizeableDialog::AddAnchor(const resizeabledlghelper::ANCHOR& anchor)
{
	_anchors.push_back(anchor);
	_anchorOffsets.emplace_back(RECT());
}

void ResizeableDialog::AddAnchor(resizeabledlghelper::ANCHOR&& anchor)
{
	_anchors.emplace_back(std::move(anchor));
	_anchorOffsets.emplace_back(RECT());
}

}
}
}