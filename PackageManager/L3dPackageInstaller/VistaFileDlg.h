#pragma once

#include <Windows.h>
#include <Shobjidl.h>

#include <string>
#include <vector>

#include <unique_handle.h>

#include "ComHelper.h"

/**
 * File Dialog im Vista-Style (Common Item Dialog)
 */
class VistaFileDlg
{
public:
	explicit VistaFileDlg(const IID& rclsid);
	virtual ~VistaFileDlg(void);

	FILEOPENDIALOGOPTIONS GetOptions();
	HRESULT SetOptions(const FILEOPENDIALOGOPTIONS& options);

	HRESULT SetAllowMultiSelect(bool bAllow);
	HRESULT SetPathAndFileMustExist(bool bMustExist);

	HRESULT SetTitle(const std::wstring& title);

	HRESULT SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& filterSpec);

	virtual HRESULT Show(HWND hwndOwner);

	virtual std::vector<std::wstring> GetResult();

protected:
	CComPtr<IFileDialog> fileDlg;
	HRESULT resultShow;

private:
	//typedef HRESULT (CALLBACK* FA_SHCreateItemFromParsingName)(PCWSTR, IBindCtx*, REFIID, void**);

	//HMODULE libShell32;
	//FA_SHCreateItemFromParsingName procAddress_SHCreateItemFromParsingName;

private:
	VistaFileDlg(const VistaFileDlg& other);
	VistaFileDlg& operator=(const VistaFileDlg& other);
};
