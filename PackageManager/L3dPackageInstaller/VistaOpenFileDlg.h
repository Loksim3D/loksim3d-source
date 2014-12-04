#pragma once

#include "VistaFileDlg.h"

#include <string>
#include <vector>

#include "OpenFileDlg.h"

/**
 * Datei-Öffnen Dialog im Vista Style (Common Items Dialog)
 */
class VistaOpenFileDlg : public VistaFileDlg, public OpenFileDlg
{
public:
	VistaOpenFileDlg(void);
	virtual ~VistaOpenFileDlg(void);

	virtual std::vector<std::wstring> GetResult();

	HRESULT SetAllowMultiSelect(bool bAllow) { return VistaFileDlg::SetAllowMultiSelect(bAllow); }
	HRESULT SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& filterSpec) { return VistaFileDlg::SetFileTypes(filterSpec); }
	HRESULT SetPathAndFileMustExist(bool bMustExist) { return VistaFileDlg::SetPathAndFileMustExist(bMustExist); }
	HRESULT SetTitle(const std::wstring& title) { return VistaFileDlg::SetTitle(title); }
	HRESULT Show(HWND hwndOwner) { return VistaFileDlg::Show(hwndOwner); }

private:
	VistaOpenFileDlg(const VistaOpenFileDlg& other);
	VistaOpenFileDlg& operator=(const VistaOpenFileDlg& other);
};

