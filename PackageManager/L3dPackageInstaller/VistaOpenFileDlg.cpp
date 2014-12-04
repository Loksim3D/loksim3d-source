#include "StdAfx.h"
#include "VistaOpenFileDlg.h"



using namespace std;

VistaOpenFileDlg::VistaOpenFileDlg(void) : VistaFileDlg(CLSID_FileOpenDialog)
{
}


VistaOpenFileDlg::~VistaOpenFileDlg(void)
{
}

vector<wstring> VistaOpenFileDlg::GetResult()
{
	if (SUCCEEDED(resultShow) && ((GetOptions() & FOS_ALLOWMULTISELECT) != 0))
	{
		vector<wstring> ret;
		HRESULT hr;
		CComPtr<IFileOpenDialog> pOpenDlg;

		if (SUCCEEDED(hr = fileDlg->QueryInterface(IID_PPV_ARGS(&pOpenDlg))))
		{	
			CComPtr<IShellItemArray> pResults;

			if (SUCCEEDED(hr = pOpenDlg->GetResults(&pResults)))
			{
				DWORD cSelItems;

				// Get the number of selected files.				
				if (SUCCEEDED(hr = pResults->GetCount(&cSelItems)))
				{
					for (DWORD j = 0; j < cSelItems; j++)
					{
						CComPtr<IShellItem> pItem;

						// Get an IShellItem interface on the next file.						
						if (SUCCEEDED(hr = pResults->GetItemAt (j, &pItem)))
						{
							LPOLESTR pwsz = NULL; 

							// Get its file system path.
							if (SUCCEEDED(hr = pItem->GetDisplayName (SIGDN_FILESYSPATH, &pwsz)))
							{
								ret.push_back(pwsz);
								CoTaskMemFree (pwsz);
							}
						}
					}
				}
			}
		}
		return ret;
	}
	else
	{
		return VistaFileDlg::GetResult();
	}
}