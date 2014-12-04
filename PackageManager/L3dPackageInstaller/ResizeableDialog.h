#pragma once
#include "Dialog.h"
#include "ResizeableDialogHelperTypes.h"
#include <vector>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{


class ResizeableDialog : virtual public Dialog
{
public:
	ResizeableDialog(void);
	~ResizeableDialog(void);

protected:
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnSize(WPARAM wParam, LPARAM lParam) override;

	void AddAnchor(resizeabledlghelper::ANCHOR&& anchor);
	void AddAnchor(const resizeabledlghelper::ANCHOR& anchor);

private:
	std::vector<resizeabledlghelper::ANCHOR> _anchors;
	std::vector<RECT> _anchorOffsets;
	bool _initialized;
};


}
}
}