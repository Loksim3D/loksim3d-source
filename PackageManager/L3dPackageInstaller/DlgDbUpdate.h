#pragma once

#include <future>
#include <vector>

#include "Dialog.h"

namespace l3d
{
	namespace packageinstaller
	{
		namespace ui
		{

			class DlgDbUpdate :
				public Dialog
			{
			public:
				explicit DlgDbUpdate(std::wstring l3dPath);

				INT_PTR Show(HINSTANCE hInstance, HWND hWndParent);
				virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam) override;
				virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam) override;
				virtual INT_PTR DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam) override;

			private:
				const std::wstring l3dPath_;
				std::future<void> updateTask_;
			};


		}
	}
}