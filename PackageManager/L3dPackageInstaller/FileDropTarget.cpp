#include "StdAfx.h"
#include "FileDropTarget.h"
#include <Windows.h>
#include <ShlObj.h>
#include <Shellapi.h>
#include <algorithm>
#include <boost/filesystem.hpp>

namespace l3d
{
namespace packageinstaller
{

	FileDropTarget::FileDropTarget(std::vector<std::wstring> exts) : acceptedExtensions(std::move(exts)), onDropHandler{nullptr}
{
}


void FileDropTarget::SetOnDropNotify(IDropNotifyable* notifyOnDrop)
{
	this->onDropHandler = notifyOnDrop;
}

HRESULT __stdcall FileDropTarget::DragEnter(IDataObject *pDataObject, DWORD grfKeyState,
											  POINTL pt, DWORD *pdwEffect)
{
	// does the dataobject contain data we want?
	m_fAllowDrop = QueryDataObject(pDataObject);
	
	if(m_fAllowDrop)
	{
		// get the dropeffect based on keyboard state
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);

		SetFocus(m_hWnd);

		//PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

bool FileDropTarget::QueryDataObject(IDataObject *pDataObject)
{
	FORMATETC fmtetc = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	
	OutputDebugString(L"InstallPkgDropTarget::QueryDataObject");
	// does the data object support CF_TEXT using a HGLOBAL?
	if (pDataObject->QueryGetData(&fmtetc) == S_OK)
	{
		filenames.clear();
		//FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
		fmtetc.cfFormat = CF_HDROP;
		fmtetc.ptd = NULL;
		fmtetc.dwAspect = DVASPECT_CONTENT;
		fmtetc.lindex = -1;
		fmtetc.tymed = TYMED_HGLOBAL;
		STGMEDIUM stgmed;
		TCHAR szFileName[_MAX_PATH + 1];

		HRESULT hr;
		if(SUCCEEDED(hr = pDataObject->GetData(&fmtetc, &stgmed)))
		{
			HDROP hdrop = (HDROP)GlobalLock(stgmed.hGlobal);
			if (nullptr != hdrop)
			{
				UINT nFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);

				for(UINT nNames = 0; nNames < nFiles; nNames++)
				{
					ZeroMemory(szFileName, _MAX_PATH + 1);
					DragQueryFile
						(hdrop, nNames, (LPTSTR)szFileName, _MAX_PATH + 1);
					filenames.push_back(szFileName);
				}
				GlobalUnlock(hdrop);
			}
			ReleaseStgMedium(&stgmed);
			
			if (acceptedExtensions.empty()) {
				return true;
			}
			auto unkownFile = std::find_if(begin(filenames), end(filenames), [this] (const std::wstring& name) -> bool {
				boost::filesystem::path p = name;
				auto e = p.extension().wstring();

				return std::find_if(begin(acceptedExtensions), end(acceptedExtensions), [&e](const std::wstring& el) {
					return e == el;
				}) == end(acceptedExtensions);
			});
			return unkownFile == filenames.end() && nullptr != hdrop;
		}
	}
	return false;
}

DWORD FileDropTarget::DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed)
{
	DWORD dwEffect = 0;

	dwEffect = dwAllowed & DROPEFFECT_COPY;
	
	return dwEffect;
}

HRESULT __stdcall FileDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
	if(m_fAllowDrop)
	{
		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
		//PositionCursor(m_hWnd, pt);
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

HRESULT __stdcall FileDropTarget::DragLeave(void)
{
	return S_OK;
}

HRESULT __stdcall FileDropTarget::Drop(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	//PositionCursor(m_hWnd, pt);

	if(m_fAllowDrop)
	{

		*pdwEffect = DropEffect(grfKeyState, pt, *pdwEffect);
		if (onDropHandler != nullptr)
		{
			onDropHandler->OnFilesDrop();
		}
	}
	else
	{
		*pdwEffect = DROPEFFECT_NONE;
	}
	
	return S_OK;
}

}
}