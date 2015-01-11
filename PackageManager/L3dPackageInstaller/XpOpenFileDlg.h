#pragma once

#include <Windows.h>
#include <Shobjidl.h>
#include <Commdlg.h>

#include <memory>

#include "OpenFileDlg.h"

/**
 * Datei-Öffnen Dialog im Windows XP Style
 */
class XpOpenFileDlg : public OpenFileDlg
{
public:
	XpOpenFileDlg(void);
	virtual ~XpOpenFileDlg(void);

	HRESULT SetAllowMultiSelect(bool bAllow);
	HRESULT SetPathAndFileMustExist(bool bMustExist);
	
	HRESULT SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& filterSpec);
	
	HRESULT SetTitle(const std::wstring& title);

	HRESULT Show(HWND hwndOwner);

	virtual std::vector<std::wstring> GetResults() override;

	HRESULT SetDefaultExtension(const std::wstring& extension) { return E_NOTIMPL; }

private:
	OPENFILENAME options;
	std::unique_ptr<wchar_t[]> filterBuf;
	std::unique_ptr<wchar_t[]> filenameBuf;
	std::wstring dlgTitle;

private:
	XpOpenFileDlg(const XpOpenFileDlg&);
	XpOpenFileDlg& operator=(const XpOpenFileDlg&);
};

