#pragma once
#include "ResizeableDialog.h"
#include "DlgPackageInstall.h"
#include "DlgDeinstallSelectPkg.h"
#include "DlgFileDependencies.h"
#include "DlgUndo.h"

#include "BackgroundOperatorDlg.h"

#include <lhWinAPI.h>
#include <windows.h>

#include <string>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Hauptdialog (verwaltet Tab-Control + Menü)
 */
class MainDlg :
	public ResizeableDialog
{
public:
	MainDlg(const std::wstring& l3dPath, const std::vector<std::wstring>& packagesToInstall);
	~MainDlg(void);

	INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);

protected:
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnNotify(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnClose(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnDestroy(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnSize(WPARAM wParam, LPARAM lParam) override;

private:
	DlgPackageInstall dlgInstall;
	DlgDeinstallSelectPkg dlgDeinstall;
	DlgUndo dlgUndo;
	DlgFileDependencies dlgDependencies;

	lhWinAPI::StringTable strTable;
	BackgroundOperatorDlg* curTabDlg;
	std::wstring l3dPath;
};

}
}
}