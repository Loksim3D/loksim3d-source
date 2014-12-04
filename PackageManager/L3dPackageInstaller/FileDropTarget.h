#pragma once

#include "oleidl.h"
#include <vector>
#include <string>


namespace l3d
{
namespace packageinstaller
{

class IDropNotifyable
{
public:
	virtual void OnFilesDrop() = 0;
};

/**
 * Klasse welche als "DropTarget" für Packages (d.h. Dateinamen)agiert
 */
class FileDropTarget :
	public IDropTarget
{
public:
	FileDropTarget(std::vector<std::wstring> acceptedExtensions);

public:
	/**
	 * Setzt Listener welcher aufgerufen wird, wenn eine Datei "gedropt" wird
	 */
	void SetOnDropNotify(IDropNotifyable* notifyOnDrop);
	inline const std::vector<std::wstring>& getFilenames() const { return filenames; }

    // IUnknown implementation
	HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject) { return E_FAIL; }
	ULONG   __stdcall AddRef (void) { return ++m_lRefCount; }
	ULONG   __stdcall Release (void) { return --m_lRefCount; }

    // IDropTarget implementation
    HRESULT __stdcall DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    HRESULT __stdcall DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
    HRESULT __stdcall DragLeave(void);
    HRESULT __stdcall Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);

private:

    // internal helper function
    DWORD DropEffect(DWORD grfKeyState, POINTL pt, DWORD dwAllowed);
    bool  QueryDataObject(IDataObject *pDataObject);

    // Private member variables
    long   m_lRefCount;
    HWND   m_hWnd;
    bool   m_fAllowDrop;
	std::vector<std::wstring> filenames;
	std::vector<std::wstring> acceptedExtensions;

	IDropNotifyable *onDropHandler;

    // Other internal window members
    
};

}
}