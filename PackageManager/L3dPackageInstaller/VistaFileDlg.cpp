#include "StdAfx.h"
#include "VistaFileDlg.h"

#include <Windows.h>
#include <ShlObj.h>

#include <exception>

using namespace std;

template<class T> // template used so we can avoid having to write out the cast type
bool GetCastProcAddress(HMODULE hModule, LPCSTR lpProcName, T& funcPtrToAssign) {	
	void* p = ::GetProcAddress(hModule, lpProcName);
	funcPtrToAssign = ( T ) ::GetProcAddress(hModule, lpProcName);
	return (NULL != funcPtrToAssign); // loaded okay
}

VistaFileDlg::VistaFileDlg(const IID& rclsid) : resultShow(E_FAIL)
{
	HRESULT hr = CoCreateInstance(rclsid, 
							  NULL, 
							  CLSCTX_INPROC_SERVER, 
							  IID_PPV_ARGS(&fileDlg));

	if(FAILED(hr) || fileDlg == nullptr)
	{ 
		throw exception("Vista FileDialog not supported on this OS");
	}

	SetOptions(GetOptions() | FOS_FORCEFILESYSTEM);
}


VistaFileDlg::~VistaFileDlg(void)
{
}

FILEOPENDIALOGOPTIONS VistaFileDlg::GetOptions()
{
	FILEOPENDIALOGOPTIONS ret;
	if FAILED(fileDlg->GetOptions(&ret))
	{
		throw exception("Vista FileDlg GetOptions failed");
	}

	return ret;
}

HRESULT VistaFileDlg::SetOptions(const FILEOPENDIALOGOPTIONS& options)
{
	return fileDlg->SetOptions(options);
}

HRESULT VistaFileDlg::SetAllowMultiSelect(bool bAllow)
{
	if (bAllow)
	{
		return SetOptions(GetOptions() | FOS_ALLOWMULTISELECT);
	}
	else
	{
		return SetOptions(GetOptions() & ~FOS_ALLOWMULTISELECT);
	}
}

HRESULT VistaFileDlg::SetPathAndFileMustExist(bool bMustExist)
{
	if (bMustExist)
	{
		return SetOptions(GetOptions() | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST);
	}
	else
	{
		return SetOptions(GetOptions() & ~FOS_PATHMUSTEXIST & ~FOS_FILEMUSTEXIST);
	}
}

HRESULT VistaFileDlg::SetFileTypes(const vector<COMDLG_FILTERSPEC>& filterSpec)
{
	return fileDlg->SetFileTypes(filterSpec.size(), filterSpec.data());
}

HRESULT VistaFileDlg::SetTitle(const std::wstring& title)
{
	return fileDlg->SetTitle(title.c_str());
}

HRESULT VistaFileDlg::Show(HWND hwndOwner)
{
	resultShow = fileDlg->Show(hwndOwner);
	return resultShow;
}

vector<wstring> VistaFileDlg::GetResult()
{
	HRESULT hr;
	vector<wstring> ret;
	if (SUCCEEDED(resultShow))
	{
		// Obtain the result of the user's interaction with the dialog.
		CComPtr<IShellItem> psiResult;
		hr = fileDlg->GetResult(&psiResult);

		if (SUCCEEDED(hr))
		{           
			wchar_t* path;
			hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &path);
			if (SUCCEEDED(hr))
			{
				ret.push_back(path);
				CoTaskMemFree(path);
			}
		}
	}
	return ret;
}