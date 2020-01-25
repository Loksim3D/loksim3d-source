#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#include "dialog.h"
#include <vector>
#include <map>
#include <memory>
#include <string>

#include <lhWinAPI.h>
#include "DeinstallManager.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Dialog zur Anzeige / Auswahl der Dateien bei einer Deinstallation
 */
class DlgDeInstallSelectFiles :
	public Dialog
{
public:
	DlgDeInstallSelectFiles(std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> filesToDeinstall, std::shared_ptr<std::map<int, std::wstring>> pkgIdToNameMap);
	DlgDeInstallSelectFiles(std::shared_ptr<std::map<int, std::vector<std::wstring>>> deinstalledFiles, std::shared_ptr<std::map<int, std::wstring>> pkgIdToNameMap);
	~DlgDeInstallSelectFiles(void);

	INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);

	std::shared_ptr<std::map<int, std::vector<std::wstring>>> GetSelectedFiles() { return selectedFiles; }

protected:
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnNotify(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnTimer(WPARAM wParam, LPARAM lParam) override;
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;

private:
	struct TvItemData
	{
		TvItemData(int _pkgId, const FileToDeinstall& _fileInfo) :
			pkgId(_pkgId), fileInfo(_fileInfo)
		{
		}

		int pkgId;
		FileToDeinstall fileInfo;
	};

private:
	void SetCheckStateAllChilds(HTREEITEM parent, BOOL fCheck);

	template <class Func>
	void ForAllTvItems(Func func);
	template <class Func>
	void ForAllTvChildItems(HTREEITEM treeItem, Func func);

	HWND hwndTree;
	std::vector<std::unique_ptr<TvItemData>> vecTvItemData;

	std::shared_ptr<std::map<int, std::vector<FileToDeinstall>>> filesToDeinstall;
	std::shared_ptr<std::map<int, std::vector<std::wstring>>> deinstalledFiles;

	/**
	 * Variablen in der die ausgewählten dateien gespeichert werden pro package
	 */
	std::shared_ptr<std::map<int, std::vector<std::wstring>>> selectedFiles;

	std::shared_ptr<std::map<int, std::wstring>> pkgIdToNameMap;

	lhWinAPI::StringTable strTable;
};


template <class Func>
void DlgDeInstallSelectFiles::ForAllTvItems(Func func)
{
	HTREEITEM cur = TreeView_GetRoot(hwndTree);
	while (cur != nullptr)
	{
		func(cur);
		ForAllTvChildItems(cur, func);
		cur = TreeView_GetNextItem(hwndTree, cur, TVGN_NEXT);
	}	
}

template <class Func>
void DlgDeInstallSelectFiles::ForAllTvChildItems(HTREEITEM treeItem, Func func)
{
	HTREEITEM child = TreeView_GetNextItem(hwndTree, treeItem, TVGN_CHILD);
	HTREEITEM cur = child;
	while (cur != nullptr)
	{
		func(cur);
		ForAllTvChildItems(cur, func);
		cur = TreeView_GetNextItem(hwndTree, cur, TVGN_NEXT);
	}	
}

}
}
}