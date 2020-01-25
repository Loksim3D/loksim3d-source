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

	virtual std::vector<std::wstring> GetResults() override;

	virtual HRESULT SetAllowMultiSelect(bool bAllow) override { return VistaFileDlg::SetAllowMultiSelect(bAllow); }
	virtual HRESULT SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& filterSpec) override  { return VistaFileDlg::SetFileTypes(filterSpec); }
	virtual HRESULT SetPathAndFileMustExist(bool bMustExist) override  { return VistaFileDlg::SetPathAndFileMustExist(bMustExist); }
	virtual HRESULT SetTitle(const std::wstring& title) override  { return VistaFileDlg::SetTitle(title); }
	virtual HRESULT Show(HWND hwndOwner) override  { return VistaFileDlg::Show(hwndOwner); }
	virtual HRESULT SetDefaultExtension(const std::wstring& extension) override  { return VistaFileDlg::SetDefaultExtension(extension); }

	virtual HRESULT SetClientGuid(const GUID& guid) override { return VistaFileDlg::SetClientGuid(guid); }

private:
	VistaOpenFileDlg(const VistaOpenFileDlg& other);
	VistaOpenFileDlg& operator=(const VistaOpenFileDlg& other);
};

