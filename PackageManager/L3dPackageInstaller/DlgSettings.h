#pragma once
#include "Dialog.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Dialog für Einstellungen
 */
class DlgSettings :
	public Dialog
{
public:
	DlgSettings(void);
	~DlgSettings(void);

	INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);

protected:
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
};


}
}
}

