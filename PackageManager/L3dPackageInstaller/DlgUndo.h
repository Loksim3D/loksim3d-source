#pragma once

#include "BackgroundOperatorDlg.h"
#include "ResizeableDialog.h"

#include <boost/filesystem.hpp>
#include <lhWinAPI.h>
#include <vector>
#include <memory>
#include "DeinstallManager.h"

#include "DBUnOrInstallPkgInfo.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Dialog fürs Rückgängigmachen von Installationen
 */
class DlgUndo :
		public BackgroundOperatorDlg, public ResizeableDialog
{
public:
	DlgUndo(const boost::filesystem::path& l3dPath);
	virtual ~DlgUndo(void);

	HWND CreateModeless(HINSTANCE hInstance, HWND hWndParent);

protected:
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnNotify(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnTimer(WPARAM wParam, LPARAM lParam) override;

	virtual INT_PTR OnSize(WPARAM wParam, LPARAM lParam) override;

	virtual INT_PTR DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam) override;

	virtual bool IsOperationRunning() const override;

	virtual bool SoftCancel() override;

private:
	/**
	 * InitListViewColumns: Adds columns to a list-view control.
	 * @returns true if successful, and false otherwise. 
	 */
	bool InitListViewColumns();
	bool InsertListViewItems(HWND listView, size_t cntItems);
	void SearchSelectableItems();

private:
	boost::filesystem::path l3dPath;
	HWND hListSelectablePkgs;
	lhWinAPI::StringTable strTable;
	std::vector<db::DBUnOrInstallPkgInfo> displayedItems;

	std::unique_ptr<DeinstallManager> deinstallMgmt;
	//std::unique_ptr<UndoDeinstallAgent> undoMgmt;
};

}
}
}