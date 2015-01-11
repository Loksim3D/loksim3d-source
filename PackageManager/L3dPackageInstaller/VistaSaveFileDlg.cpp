#include "StdAfx.h"
#include "VistaSaveFileDlg.h"

using namespace std;

VistaSaveFileDlg::VistaSaveFileDlg(void) : VistaFileDlg(CLSID_FileSaveDialog)
{
}

wstring VistaSaveFileDlg::GetResult()
{
	return VistaFileDlg::GetResult();
}