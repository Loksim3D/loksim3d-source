#include "StdAfx.h"
#include "FileDlgHelper.h"

using namespace std;

unique_ptr<OpenFileDlg> GetOpenFileDlg()
{
	static bool tryVista = true;
	unique_ptr<OpenFileDlg> dlg;
	if (tryVista)
	{
		try
		{
			dlg.reset(new VistaOpenFileDlg());
		}
		catch (...)
		{
			tryVista = false;
			dlg.reset(new XpOpenFileDlg());
		}
	}
	else
	{
		dlg.reset(new XpOpenFileDlg());
	}
	return dlg;
}