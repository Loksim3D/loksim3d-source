#pragma once

#include "VistaFileDlg.h"

#include <string>
#include <vector>

#include "SaveFileDlg.h"

/**
 * Datei-Speichern Dialog im Vista Style (Common Items Dialog)
 */
class VistaSaveFileDlg : public VistaFileDlg, public SaveFileDlg
{
public:
	VistaSaveFileDlg(void);
	VistaSaveFileDlg(const VistaSaveFileDlg& other) = delete;
	VistaSaveFileDlg& operator=(const VistaSaveFileDlg& other) = delete;

	virtual std::wstring GetResult() override;

	virtual HRESULT SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& filterSpec) override  { return VistaFileDlg::SetFileTypes(filterSpec); }
	virtual HRESULT SetPathAndFileMustExist(bool bMustExist) override  { return VistaFileDlg::SetPathAndFileMustExist(bMustExist); }
	virtual HRESULT SetTitle(const std::wstring& title) override  { return VistaFileDlg::SetTitle(title); }
	virtual HRESULT Show(HWND hwndOwner) override { return VistaFileDlg::Show(hwndOwner); }
	virtual HRESULT SetDefaultExtension(const std::wstring& extension) override  { return VistaFileDlg::SetDefaultExtension(extension); }

	virtual HRESULT SetClientGuid(const GUID& guid) override { return VistaFileDlg::SetClientGuid(guid); }
};

