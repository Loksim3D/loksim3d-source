#pragma once

#include <Windows.h>
#include <boost/noncopyable.hpp>

namespace l3d
{
namespace packageinstaller
{
namespace ui
{

/**
 * Einfache Kapselung eines Win32 Dialogs
 */
class Dialog : boost::noncopyable
{
public:
	HWND GetHwnd() { return hDlg; }

	/**
	 * Setzt minimale größe des Dialogs
	 * -1 bedeutet beliebige minimal größe (kann bei x und/oder y gesetzt werden)
	 */
	void SetMinSize(const POINT& _minSize) { minSize = _minSize; }

protected:
	Dialog(void);
	virtual ~Dialog(void);

	/**
	 * Zeit den Dialog an<br>
	 * siehe DialogBoxParam für Parameterbeschreibung
	 */
	INT_PTR Show(__in_opt HINSTANCE hInstance,
				 __in LPCWSTR lpTemplateName,
				 __in_opt HWND hWndParent);

	/**
	 * Erzeugt einen modeless dialog<br>
	 * siehe CreateDialogParam für Parameterbeschreibung
	 */
	HWND CreateModeless(__in_opt  HINSTANCE hInstance,
						__in      LPCTSTR lpTemplateName,
						__in_opt  HWND hWndParent);

	/**
	 * Window-Prozedur die überschrieben werden kann<br>
	 * Jedoch sollte immer diese Methode aufgerufen werden, damit die Nachrichten an die entsprechenden anderen Methoden weitergeleitet werden
	 */
	virtual INT_PTR DlgWndProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual INT_PTR OnInitDlg(WPARAM wParam, LPARAM lParam);
	virtual INT_PTR OnCommand(WPARAM wParam, LPARAM lParam);
	virtual INT_PTR OnNotify(WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual INT_PTR OnTimer(WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual INT_PTR OnClose(WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual INT_PTR OnDestroy(WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual INT_PTR OnSize(WPARAM wParam, LPARAM lParam) { return FALSE; }
	virtual INT_PTR OnGetMinMaxInfo(WPARAM wParam, LPARAM lParam);

	HINSTANCE GetHInstance() const { return hInstance; }

private:
	static INT_PTR CALLBACK StaticWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	HWND hDlg;
	HINSTANCE hInstance;

	POINT minSize;
};

}
}
}