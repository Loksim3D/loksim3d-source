#pragma once
#include "BackgroundOperatorDlg.h"
#include "InstallManager.h"
#include "FileDropTarget.h"
#include "ResizeableDialog.h"

#include <Windows.h>

#include <string>
#include <vector>

#include <lhWinAPI.h>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

struct PkgDispInfo
{
	PkgDispInfo() : 
			finishedPreInstall(false)
	{
	}

	PkgDispInfo(const std::wstring& _completePath, const std::wstring& _dispName) :
			completePath(_completePath), dispName(_dispName), finishedPreInstall(false)
	{
	}

	std::wstring completePath;
	std::wstring dispName;
	bool finishedPreInstall;
};

/**
 * Dialog zur Auswahl der Packages die installiert werden sollen
 */
class DlgPackageInstall :
	public BackgroundOperatorDlg, public IDropNotifyable, public ResizeableDialog
{
public:
	DlgPackageInstall(const std::wstring& l3dPath, const std::vector<std::wstring>& packagesToInstall);
	virtual ~DlgPackageInstall(void);

	//INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);

	HWND CreateModeless(HINSTANCE hInstance, HWND hWndParent);

	virtual void OnFilesDrop() override;

	virtual bool IsOperationRunning() const override { return installationStarted; }

	virtual bool SoftCancel() override;

protected:
	//INT_PTR DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
	INT_PTR OnNotify(WPARAM wParam, LPARAM lParam) override;
	INT_PTR OnTimer(WPARAM wParam, LPARAM lParam) override;

	INT_PTR OnSize(WPARAM wParam, LPARAM lParam) override;

private:
	void CloseDlg(bool reuseSelectedPackages = false);
	void DoAddFiles(const std::vector<std::wstring>& filenames);
	void DoAddFilesDlg();
	void SetInstallBtnState();
	void SetUpDownBtnState();
	void OnSelectionChanged();

	std::vector<PkgDispInfo> selectedPackages;
	std::unique_ptr<InstallManager> (installManager);

	HWND hListPackages;
	HWND hTextBoxReadme;

	int curListSel;
	bool installationStarted;

	lhWinAPI::StringTable strTable;

	FileDropTarget dropTarget;

	std::vector<std::wstring> argPackagesToInstall;
};

}
}
}