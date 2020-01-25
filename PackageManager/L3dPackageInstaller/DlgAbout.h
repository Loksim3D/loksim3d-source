#pragma once
#include "Dialog.h"
#include <vector>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * About / Über Dialog
 */
class DlgAbout :
	public Dialog
{
public:
	DlgAbout(void);
	~DlgAbout(void);

	INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam) override;

private:
	void SetUrlCtrl(HWND urlCtrl);

	std::vector<HFONT> fontsToDelete;

};


}
}
}