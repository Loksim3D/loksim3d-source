#include "StdAfx.h"
#include "XpOpenFileDlg.h"

#include <algorithm>
#include <memory>
#include <sstream>

using namespace std;

XpOpenFileDlg::XpOpenFileDlg(void)
{
	ZeroMemory(&options, sizeof options);
	options.lStructSize = sizeof (OPENFILENAME);
	options.Flags |= OFN_EXPLORER;
}


XpOpenFileDlg::~XpOpenFileDlg(void)
{
}

HRESULT XpOpenFileDlg::SetAllowMultiSelect(bool bAllow)
{
	if (bAllow)
	{
		options.Flags |= OFN_ALLOWMULTISELECT;
	}
	else
	{
		options.Flags &= ~OFN_ALLOWMULTISELECT;
	}
	return S_OK;
}

HRESULT XpOpenFileDlg::SetPathAndFileMustExist(bool bMustExist)
{
	if (bMustExist)
	{
		options.Flags |= OFN_FILEMUSTEXIST;
		options.Flags |= OFN_PATHMUSTEXIST;
	}
	else
	{
		options.Flags &= ~OFN_FILEMUSTEXIST;
		options.Flags &= ~OFN_PATHMUSTEXIST;
	}
	return S_OK;
}


HRESULT XpOpenFileDlg::SetFileTypes(const vector<COMDLG_FILTERSPEC>& filterSpec)
{
	wstringstream str;
	for_each(filterSpec.begin(), filterSpec.end(), [&str](const COMDLG_FILTERSPEC& f) {
		str << f.pszName << '\0' << f.pszSpec << '\0';
	});
	str << '\0';

	wstring s = str.str();
	size_t si = s.size();
	filterBuf.reset(new wchar_t[si]);
	wmemcpy_s(filterBuf.get(), si, s.c_str(), si);
	options.lpstrFilter = filterBuf.get();
	return S_OK;
}

HRESULT XpOpenFileDlg::SetTitle(const std::wstring& title)
{
	dlgTitle = title;
	options.lpstrTitle = dlgTitle.c_str();
	return S_OK;
}

HRESULT XpOpenFileDlg::Show(HWND hwndOwner)
{	
	options.hwndOwner = hwndOwner;

	if ((options.Flags & OFN_ALLOWMULTISELECT) != 0)
	{
		filenameBuf.reset(new wchar_t[MAX_PATH * 20]);
		options.nMaxFile = MAX_PATH * 20;
	}
	else
	{
		filenameBuf.reset(new wchar_t[MAX_PATH * 2]);
		options.nMaxFile = MAX_PATH * 2;
	}
	filenameBuf.get()[0] = '\0';
	options.lpstrFile = filenameBuf.get();
	if (GetOpenFileName(&options))
	{
		return S_OK;
	}

	return E_FAIL;
}

vector<wstring> XpOpenFileDlg::GetResults()
{
	vector<wstring> ret;
	if (filenameBuf)
	{
		if ((options.Flags & OFN_ALLOWMULTISELECT) == 0)
		{
			ret.push_back(filenameBuf.get());
		}
		else
		{
			std::wstring dir = filenameBuf.get();
			wchar_t *curFile = filenameBuf.get() + dir.length() + 1;

			while (lstrlen(curFile) > 0)
			{
				ret.push_back(dir + L"\\" + curFile);
				curFile = curFile + lstrlen(curFile) + 1;
			}
			if (ret.empty())
			{
				// Nur eine Datei ausgewählt => keine explizite Trennung von Datei- und Pfadname
				ret.push_back(dir);
			}
		}
	}
	return ret;
}

