#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#include "dialog.h"
#include <vector>
#include <map>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>

#include <lhWinAPI.h>
#include "InstallManager.h"

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

struct TvItemData
{
	TvItemData(const std::wstring& _packageName, const insthelper::FileInstallInfo& _fileInfo) :
		packageName(_packageName), fileInfo(_fileInfo)
	{
	}

	TvItemData(const std::wstring& _packageName, const insthelper::FileInstallInfo& _fileInfo, int _pkgId) :
		packageName(_packageName), fileInfo(_fileInfo), pkgId(_pkgId)
	{
	}

	std::wstring packageName;
	insthelper::FileInstallInfo fileInfo;

	int pkgId;
};

/**
 * Dialog welcher bei Packageinstallationen die zu installierenden Dateien anzeigt bzw Auswahl ermöglicht welche Dateien installiert werden sollen
 */
class DlgInstallSelectFiles :
	public Dialog
{
public:
	DlgInstallSelectFiles(void) { }
	DlgInstallSelectFiles(const std::map<std::wstring, std::unique_ptr<insthelper::InstallInformation>>* fileMap, 
		std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> deinstalledFiles, bool userShouldSelect);

	DlgInstallSelectFiles(const std::map<std::wstring, std::unique_ptr<insthelper::InstallInformation>>* fileMap, 
		std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> deinstalledFiles, bool userShouldSelect,
		std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> selectedFiles);

	~DlgInstallSelectFiles(void);

	INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);

	/**
	 * Liefert die Auswahl des Benutzers
	 */
	std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> GetSelectedFiles() { return selectedFiles; }

	/**
	 * Setzt den Pfad zum L3D-Verzeichnis:
	 * wird benötigt um installierte Dateien öffnen zu können
	 */
	void SetL3dPath(const boost::filesystem::path& l3dPath) { this->l3dPath = l3dPath; }

	/**
	 * Setzt eine UserSelection welche die angezeigten Dateien einschränkt
	 */
	void SetDisplayOnlyUserSelection(const std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>>& userSel) { selectedFiles = userSel; onlyDisplayUserSelection = true; }

protected:
	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam);
	virtual INT_PTR OnNotify(WPARAM wParam, LPARAM lParam);
	virtual INT_PTR OnTimer(WPARAM wParam, LPARAM lParam);
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);

private:
	void SetCheckStateAllChilds(HTREEITEM parent, BOOL fCheck);

	void ExportInstallInformation(const std::wstring& exportToFile);
	void WriteFileToExportList(const insthelper::FileInstallInfo& fii, std::wofstream& os);

	template <class Func>
	void ForAllTvItems(Func func);
	template <class Func>
	void ForAllTvChildItems(HTREEITEM treeItem, Func func);


	HWND hwndTree;
	const std::map<std::wstring, std::unique_ptr<insthelper::InstallInformation>>* fileMap;
	std::vector<std::unique_ptr<TvItemData>> vecTvItemData;

	/**
	 * Variablen in der die ausgewählten dateien gespeichert werden pro package
	 */
	std::shared_ptr<std::map<std::wstring, insthelper::UserSelection>> selectedFiles;

	bool userShouldSelect;

	lhWinAPI::StringTable strTable;

	boost::filesystem::path l3dPath;

	// Falls true werden nur jene Elemente in die TV eingefügt, die in 'selectedFiles' ausgewählt sind
	bool onlyDisplayUserSelection;

	std::shared_ptr<std::vector<std::pair<db::DBPackageEntry, std::vector<std::wstring>>>> deinstalledFiles;
};


template <class Func>
void DlgInstallSelectFiles::ForAllTvItems(Func func)
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
void DlgInstallSelectFiles::ForAllTvChildItems(HTREEITEM treeItem, Func func)
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