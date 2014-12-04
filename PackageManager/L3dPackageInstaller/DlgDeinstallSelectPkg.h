#pragma once
#include <Windows.h>
#include <lhWinAPI.h>
#include <vector>
#include <memory>
#include <map>
#include <utility>
#include <boost/filesystem.hpp>


#include "BackgroundOperatorDlg.h"
#include "ResizeableDialog.h"

#include "PackageSearch.h"
#include "DBPackageEntry.h"
#include "DeinstallManager.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Dialog zur Auswahl der Packages  die deinstalliert werden sollen
 */
class DlgDeinstallSelectPkg :
	public BackgroundOperatorDlg, public ResizeableDialog
{
public:
	explicit DlgDeinstallSelectPkg(const boost::filesystem::path& l3dPath);
	~DlgDeinstallSelectPkg(void);


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
	void SortSelectableItems();
	void FilterSelectableItems();
	void SetSortHeader();

private:
	boost::filesystem::path l3dPath;

	HWND hListSelectablePkgs;
	HWND hListSelectedPkgs;
	l3d::packageinstaller::db::PackageSearch pkgSearch;
	lhWinAPI::StringTable strTable;
	std::vector<db::DBPackageEntry> selectableItems;
	std::vector<db::DBPackageEntry> selectedItems;

	std::unique_ptr<DeinstallManager> deinstallManager;

	std::shared_ptr<std::map<int, std::wstring>> pkgIdToNameMap;

	void ResetDialog();

	/**
	 * Definiert nach welcher Spalte gerade sortiert ist (.first) bzw welche Richtung (true ist absteigend)
	 */
	std::pair<int, bool> sortColDir;

	bool gatheredFiles;
};


}
}
}