#pragma once

#include <lhWinAPI.h>

#include "BackgroundOperatorDlg.h"
#include "FileDropTarget.h"
#include "FileSearch.h"
#include "ResizeableDialog.h"

namespace l3d
{
	namespace packageinstaller
	{
		namespace ui
		{

			class DlgFileDependencies :
				public BackgroundOperatorDlg, public IDropNotifyable, public ResizeableDialog
			{
			public:
				explicit DlgFileDependencies(std::wstring l3dPath);
				~DlgFileDependencies();

				HWND CreateModeless(HINSTANCE hInstance, HWND hWndParent);

				virtual void OnFilesDrop() override final;

				virtual bool IsOperationRunning() const override final { return false; }
				virtual bool SoftCancel() override final { return true; }

			protected:
				virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override final;
				virtual INT_PTR OnSize(WPARAM wParam, LPARAM lParam) override final;
				virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override final;
				virtual INT_PTR OnNotify(WPARAM wParam, LPARAM lParam) override final;
				virtual INT_PTR DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam) override final;

			private:
				void SearchFiles();
				void FillDependencyList();

				bool InsertListViewItems(HWND listView, size_t cntItems);
				void SetUrlCtrl(HWND urlCtrl);

				FileDropTarget dropTarget_;
				std::wstring l3dPath_;

				db::FileSearch fileSearch_;

				std::vector<db::DBFileEntry> files_;
				std::vector<std::pair<db::DBDependencyEntry, db::DBPackageEntry>> dependencies_;

				HWND hTxtFilter_;
				HWND hListFiles_;
				HWND hListDependencies_;

				lhWinAPI::StringTable strTable_;

				int curFile_;
				int curDependency_;
				std::wstring lastSearch_;

				std::vector<HFONT> fontsToDelete_;
			};

		}
	}
}