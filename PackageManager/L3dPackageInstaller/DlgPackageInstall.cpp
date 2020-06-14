#include "StdAfx.h"
#include "DlgPackageInstall.h"
#include "L3dPackageInstaller.h"
#include "FileDlgHelper.h"
#include "InstallerException.h"
#include "DlgInstallSelectFiles.h"

#include <Strsafe.h>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

#include <boost/filesystem.hpp>
#include <lhWinAPI.h>

#include "DBHelper.h"


namespace l3d
{
namespace packageinstaller
{
namespace ui
{

using namespace std;
using namespace db;

DlgPackageInstall::DlgPackageInstall(const std::wstring& l3dPath, const std::vector<std::wstring>& packagesToInstall) : 
	curListSel(-1), installationStarted(false), hListPackages(nullptr), 
	dropTarget{ { L".zip", L".l3dpack" } }, argPackagesToInstall(packagesToInstall)

{
	bool disableTransactions = DBHelper::instance().GetPreferenceValue(DBHelper::PREF_DISABLE_TRANSACTIONS, false);
	installManager = std::make_unique<InstallManager>(l3dPath, disableTransactions);
	dropTarget.SetOnDropNotify(this);
}


DlgPackageInstall::~DlgPackageInstall(void)
{
	if (hListPackages != nullptr)
	{
		RevokeDragDrop(hListPackages);
	}
}

//INT_PTR DlgPackageInstall::Show(HINSTANCE hInstance, HWND hWndParent)
//{
//	return Dialog::Show(hInstance, MAKEINTRESOURCE(IDD_INSTALLPACKAGES), hWndParent);
//}

HWND DlgPackageInstall::CreateModeless(HINSTANCE hInstance, HWND hWndParent)
{
	return Dialog::CreateModeless(hInstance, MAKEINTRESOURCE(IDD_INSTALLPACKAGES), hWndParent);
}

void DlgPackageInstall::OnFilesDrop()
{
	DoAddFiles(dropTarget.getFilenames());
}

void DlgPackageInstall::DoAddFilesDlg()
{
	unique_ptr<OpenFileDlg> dlg = GetOpenFileDlg();

	dlg->SetAllowMultiSelect(true);
	vector<COMDLG_FILTERSPEC> filter(1);
	std::wstring filterName = strTable.LoadS(IDS_INSTALLPKGFILEFILTERDESC);
	std::wstring filterExt = strTable.LoadS(IDS_INSTALLPKGFILEFILTEREXT);
	filter[0].pszName = filterName.c_str();
	filter[0].pszSpec = filterExt.c_str();

	static const GUID dlgGuid =	{ 0x21f280a6, 0x5d5b, 0x44c0, { 0x97, 0x67, 0x2, 0x30, 0x94, 0xed, 0x9c, 0xbb } };
	dlg->SetClientGuid(dlgGuid);
	
	dlg->SetFileTypes(filter);
	if (SUCCEEDED(dlg->Show(GetHwnd())))
	{
		DoAddFiles(dlg->GetResults());
	}
}

void DlgPackageInstall::DoAddFiles(const vector<wstring>& filenames)
{
	LVITEM lvI;
		
	// Initialize LVITEM members that are common to all items.
	lvI.pszText   = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
	lvI.mask      = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem  = 0;
	lvI.state     = 0;


	for (auto file = begin(filenames); file != end(filenames); ++file)
	{
		boost::filesystem::path p = *file;
		if (boost::filesystem::exists(p) && installManager->AddPackageToInstall(*file))
		{
			selectedPackages.emplace_back(PkgDispInfo(*file, p.filename().wstring()));
			size_t ind = selectedPackages.size() - 1;
			lvI.iItem  = ind;
			lvI.iImage = ind;
		
			ListView_InsertItem(hListPackages, &lvI);
		}
	}
	if (selectedPackages.size() > 1 && SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXASKALWAYS, BM_GETCHECK, 0, 0) == BST_CHECKED)
	{
		SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXASKALWAYS, BM_SETCHECK, BST_UNCHECKED, 0);
		MessageBox(GetHwnd(), strTable.Load(IDS_ALWAYSASKNOTPOSSIBLE), strTable.Load(IDS_FENCY_APP_TITLE), MB_OK | MB_ICONINFORMATION);
	}
	SetInstallBtnState();
}

void DlgPackageInstall::SetInstallBtnState()
{
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONSTARTINSTALLATION), !selectedPackages.empty());
	EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXASKALWAYS), selectedPackages.empty() || selectedPackages.size() == 1);
}

void DlgPackageInstall::SetUpDownBtnState()
{
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONUP), curListSel > 0 && selectedPackages.size() > 0);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONDOWN), curListSel >= 0 && curListSel < static_cast<int>(selectedPackages.size()) - 1);
}

INT_PTR DlgPackageInstall::OnInitDlg(WPARAM wParam, LPARAM lParam)
{
	hListPackages = GetDlgItem(GetHwnd(), IDC_LISTPACKAGES);
	hTextBoxReadme = GetDlgItem(GetHwnd(), IDC_EDITPACKAGEINFO);

	RegisterDragDrop(hListPackages, &dropTarget);
	SetTimer(GetHwnd(), 0, 1000, nullptr);

	SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXONLYOLDER, BM_SETCHECK, BST_CHECKED, 0);

	SendDlgItemMessage(GetHwnd(), IDC_BUTTONUP, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(LoadIcon(GetHInstance(), MAKEINTRESOURCE(IDI_GOUP))));
	SendDlgItemMessage(GetHwnd(), IDC_BUTTONDOWN, BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(LoadIcon(GetHInstance(), MAKEINTRESOURCE(IDI_GODOWN))));

	/*installManager->AddPackageToInstall(L"D:\\new_inst_pkg.l3dpack");
	boost::filesystem::path p(L"D:\\new_inst_pkg.l3dpack");
	selectedPackages.emplace_back(PkgDispInfo(L"D:\\new_inst_pkg.l3dpack", p.filename().wstring()));
	size_t ind = selectedPackages.size() - 1;
	LVITEM lvI;
	// Initialize LVITEM members that are common to all items.
	lvI.pszText   = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
	lvI.mask      = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
	lvI.stateMask = 0;
	lvI.iSubItem  = 0;
	lvI.state     = 0;
	lvI.iItem  = ind;
	lvI.iImage = ind;
	ListView_InsertItem(hListPackages, &lvI);*/
	SetInstallBtnState();
	SetUpDownBtnState();

	SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXASKALWAYS, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_INSTALL_ALWAYS_ASK, false) ? BST_CHECKED : BST_UNCHECKED, 0);
	SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXONLYOLDER, BM_SETCHECK, 
		DBHelper::instance().GetPreferenceValue(DBHelper::PREF_INSTALL_ONLY_OLDER, true) ? BST_CHECKED : BST_UNCHECKED, 0);

	DoAddFiles(argPackagesToInstall);

	using resizeabledlghelper::ANCHOR;
	AddAnchor(ANCHOR(IDC_BUTTONADDPACKAGES,		resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_STATICPKGINFO,			resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT));

	AddAnchor(ANCHOR(IDC_BUTTONDOWN,			resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_BUTTONUP,				resizeabledlghelper::AF_TOP | resizeabledlghelper::AF_LEFT));

	AddAnchor(ANCHOR(IDC_LISTPACKAGES,			resizeabledlghelper::AF_TOP_AND_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_EDITPACKAGEINFO,		resizeabledlghelper::AF_TOP_AND_BOTTOM | resizeabledlghelper::AF_LEFT_AND_RIGHT));

	AddAnchor(ANCHOR(IDC_BUTTONREMOVEPACKAGE,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_CHECKBOXONLYOLDER,		resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_CHECKBOXASKALWAYS,		resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	
	AddAnchor(ANCHOR(IDC_STATICPROGRESSINSTALL,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT));
	AddAnchor(ANCHOR(IDC_PROGRESSINSTALL,		resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_LEFT_AND_RIGHT));

	AddAnchor(ANCHOR(IDC_BUTTONSTARTINSTALLATION,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_RIGHT));
	AddAnchor(ANCHOR(IDC_BUTTONCANCELINSTALLATION,	resizeabledlghelper::AF_BOTTOM | resizeabledlghelper::AF_RIGHT));

	return ResizeableDialog::OnInitDlg(wParam, lParam);
}

INT_PTR DlgPackageInstall::OnSize(WPARAM wParam, LPARAM lParam)
{
	return ResizeableDialog::OnSize(wParam, lParam);
}

void DlgPackageInstall::CloseDlg(bool reuseSelectedPackages /*=false*/)
{
	installManager->CancelAndWaitForTasks();
	SetTimer(GetHwnd(), 0, 1000, nullptr);

	std::wstring p = installManager->GetL3dPath().wstring();
	installManager.reset(new InstallManager(p));

	ListView_DeleteAllItems(hListPackages);
	SetWindowText(hTextBoxReadme, L"");


	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONSTARTINSTALLATION), TRUE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXASKALWAYS), TRUE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXONLYOLDER), TRUE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONREMOVEPACKAGE), FALSE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONADDPACKAGES), TRUE);
	EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELINSTALLATION), FALSE);
	installationStarted = false;

	if (reuseSelectedPackages)
	{
		vector<wstring> files;
		transform(	begin(selectedPackages), 
					end(selectedPackages), 
					back_inserter(files),
					[] (const PkgDispInfo& disp) { return disp.completePath; });
		selectedPackages.clear();
		DoAddFiles(files);
	}
	else
	{
		selectedPackages.clear();
	}

	SendDlgItemMessage(GetHwnd(), IDC_PROGRESSINSTALL, PBM_SETPOS, 0, 0);
	SetDlgItemText(GetHwnd(), IDC_STATICPROGRESSINSTALL, L"");

	//EndDialog(GetHwnd(), 0);

}

INT_PTR DlgPackageInstall::OnCommand(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
	case IDC_BUTTONADDPACKAGES:
		DoAddFilesDlg();
		break;

	case IDC_BUTTONSTARTINSTALLATION:
		if (!installationStarted)
		{
			SetTimer(GetHwnd(), 0, 500, nullptr);
			installManager->SetAlwaysAskBeforeOverwrite(SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXASKALWAYS, BM_GETCHECK, 0, 0) == BST_CHECKED);
			installManager->SetReplaceOnlyOlder(SendDlgItemMessage(GetHwnd(), IDC_CHECKBOXONLYOLDER, BM_GETCHECK, 0, 0) == BST_CHECKED);
			installManager->StartInstallation();
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONSTARTINSTALLATION), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXASKALWAYS), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_CHECKBOXONLYOLDER), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONREMOVEPACKAGE), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONADDPACKAGES), FALSE);
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONCANCELINSTALLATION), TRUE);
			installationStarted = true;
		}
		break;

	case IDC_BUTTONCANCELINSTALLATION:
		SoftCancel();
		break;
	
	case IDC_BUTTONREMOVEPACKAGE:
		if (!installationStarted && curListSel >= 0 && curListSel < static_cast<int>(selectedPackages.size()))
		{
			if (installManager->RemovePackageToInstall(selectedPackages[curListSel].completePath))
			{
				selectedPackages.erase(selectedPackages.begin() + curListSel);
				ListView_DeleteItem(hListPackages, curListSel);
				SetInstallBtnState();
				SetUpDownBtnState();
			}
		}
		break;

	case IDC_BUTTONUP:
		if (!installationStarted && curListSel > 0 && curListSel < static_cast<int>(selectedPackages.size()))
		{
			int sel = curListSel;
			swap(selectedPackages[curListSel], selectedPackages[curListSel - 1]);
			vector<wstring>& v = installManager->GetSelectedFilesToInstall();
			swap(v[curListSel], v[curListSel - 1]);
			ListView_SetItemText(hListPackages, curListSel, 0, LPSTR_TEXTCALLBACK);
			ListView_SetItemText(hListPackages, curListSel - 1, 0, LPSTR_TEXTCALLBACK);
			ListView_SetItemState(hListPackages, curListSel, 0, 0x000F);
			curListSel = sel - 1;
			ListView_SetItemState(hListPackages, curListSel, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
			ListView_SetSelectionMark(hListPackages, curListSel);
			OnSelectionChanged();
		}
		break;

	case IDC_BUTTONDOWN:
		if (!installationStarted && curListSel >= 0 && curListSel < static_cast<int>(selectedPackages.size()) - 1)
		{
			int sel = curListSel;
			swap(selectedPackages[curListSel], selectedPackages[curListSel + 1]);
			vector<wstring>& v = installManager->GetSelectedFilesToInstall();
			swap(v[curListSel], v[curListSel + 1]);
			ListView_SetItemText(hListPackages, curListSel, 0, LPSTR_TEXTCALLBACK);
			ListView_SetItemText(hListPackages, curListSel + 1, 0, LPSTR_TEXTCALLBACK);
			ListView_SetItemState(hListPackages, curListSel, 0, 0x000F);
			curListSel = sel + 1;
			ListView_SetItemState(hListPackages, curListSel, LVIS_SELECTED | LVIS_FOCUSED, 0x000F);
			ListView_SetSelectionMark(hListPackages, curListSel);
			OnSelectionChanged();
		}
		break;
	}
	return Dialog::OnCommand(wParam,lParam);
}

void DlgPackageInstall::OnSelectionChanged()
{
	if (curListSel >= 0 && curListSel < static_cast<int>(selectedPackages.size()))
	{
		PkgDispInfo& inf = selectedPackages[curListSel];
		if (!inf.finishedPreInstall)
		{
			inf.finishedPreInstall = installManager->IsPreInstallFinished(inf.completePath);
		}
		if (inf.finishedPreInstall)
		{
			const std::wstring r = installManager->GetReadmeForPackage(inf.completePath);
			SetWindowText(hTextBoxReadme, r.c_str());
		}
		else
		{
			SetWindowText(hTextBoxReadme, strTable.Load(IDS_LOADING_README));
		}
		if (!installationStarted)
		{
			EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONREMOVEPACKAGE), TRUE);
		}
		SetUpDownBtnState();
	}
	else
	{
		EnableWindow(GetDlgItem(GetHwnd(), IDC_BUTTONREMOVEPACKAGE), FALSE);
		SetWindowText(hTextBoxReadme, L"");
		SetUpDownBtnState();
	}
}

INT_PTR DlgPackageInstall::OnNotify(WPARAM wParam, LPARAM lParam)
{
	switch (((LPNMHDR) lParam)->code)
	{
	case LVN_GETDISPINFO:
		{
			NMLVDISPINFO *plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam);

			try
			{
				if (plvdi->hdr.hwndFrom == hListPackages && (plvdi->item.mask & LVIF_TEXT) != 0)
				{
					const wstring& txt = selectedPackages.at(plvdi->item.iItem).dispName;
					StringCchCopy(plvdi->item.pszText, plvdi->item.cchTextMax, txt.c_str());
				}
			} catch (const std::out_of_range&)
			{
			}
		}
		break;
	case LVN_ITEMCHANGED:
		{
			int newSel = (int)SendMessage(hListPackages, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
			//if (newSel != curListSel)
			{
				curListSel = newSel;
				OnSelectionChanged();
			}
			/*
			LPNMLISTVIEW pnmv = reinterpret_cast<LPNMLISTVIEW>(lParam);
			if (pnmv->iItem >= 0 && pnmv->iItem < static_cast<int>(selectedPackages.size()) && 
				(pnmv->uNewState & LVIS_SELECTED) != 0 && (pnmv->uChanged & LVIF_STATE) != 0)
			{
				curListSel = pnmv->iItem;
				OnSelectionChanged();
			}
			else if ((pnmv->uNewState & LVIS_SELECTED) == 0 && (pnmv->uChanged & LVIF_STATE) != 0)
			{
				curListSel = -1;
				OnSelectionChanged();
			}
			*/
		}
	}
	return Dialog::OnNotify(wParam, lParam);
}

bool DlgPackageInstall::SoftCancel()
{
	if (installationStarted)
	{
		if (installManager->IsUsingTransaction() || MessageBox(GetHwnd(), strTable.LoadS(IDS_WARNINGCANCEL).c_str(), 
			strTable.LoadS(IDS_WARNINGCANCELTITLE).c_str(), MB_ICONWARNING | MB_YESNO) == IDYES)
		{
			installManager->CancelInstallation();
			EnableWindow(GetDlgItem(GetHwnd(), IDCANCEL), FALSE);
		}
		else
		{
			return false;
		}
	}
	return true;
}

INT_PTR DlgPackageInstall::OnTimer(WPARAM wParam, LPARAM lParam)
{
	if (curListSel >= 0 && curListSel < static_cast<int>(selectedPackages.size()))
	{
		PkgDispInfo& inf = selectedPackages[curListSel];
		if (!inf.finishedPreInstall)
		{
			inf.finishedPreInstall = installManager->IsPreInstallFinished(inf.completePath);
			if (inf.finishedPreInstall)
			{
				ListView_SetItemText(hListPackages, curListSel, 0, LPSTR_TEXTCALLBACK);
			}
		}
	}
	if (installationStarted)
	{
		size_t cur, total;
		SendDlgItemMessage(GetHwnd(), IDC_PROGRESSINSTALL, PBM_SETPOS, installManager->GetInstallationProgress(&cur, &total), 0);
		TCHAR buf[260];
		swprintf_s(buf, strTable.Load(IDS_INSTALLPROGTXT), cur, total);
		SetDlgItemText(GetHwnd(), IDC_STATICPROGRESSINSTALL, buf);
		if (installManager->IsInstallFinished())
		{
			KillTimer(GetHwnd(), 0);
			InstallManager::InstallManagerState st = installManager->GetInstallerState();
			
			if (st == InstallManager::InstallSuccessfullyFinished || st == InstallManager::InstallPassOneFinished)
			{
				std::unique_ptr<DlgInstallSelectFiles> dlg;
				if (st == InstallManager::InstallPassOneFinished)
				{
					dlg = std::unique_ptr<DlgInstallSelectFiles>(
						new DlgInstallSelectFiles(installManager->GetInstallInformation(), installManager->GetFilesToDeinstall(), true));
				}
				else if (installManager->IsAlwaysAsk())
				{
					dlg = std::unique_ptr<DlgInstallSelectFiles>(
						new DlgInstallSelectFiles(installManager->GetInstallInformation(), installManager->GetDeinstalledFiles(), false, installManager->GetUserSelection()));
				}
				else
				{
					dlg = std::unique_ptr<DlgInstallSelectFiles>(
						new DlgInstallSelectFiles(installManager->GetInstallInformation(), installManager->GetDeinstalledFiles(), false));
				}
				dlg->SetL3dPath(installManager->GetL3dPath());
				if (dlg->Show(GetHInstance(), GetHwnd()) == IDOK && st == InstallManager::InstallPassOneFinished)
				{
					installManager->StartInstallUserSelection(dlg->GetSelectedFiles());
					SetTimer(GetHwnd(), 0, 500, nullptr);
				}
				else
				{
					CloseDlg();
				}
			}
			else if (st == InstallManager::InstallCancelled)
			{
				std::wstring msg = strTable.LoadS(IDS_INSTALLCANCELLED).append(L"\n");
				if (installManager->IsUsingTransaction())
				{
					msg.append(strTable.LoadS(IDS_INSTALLROLLEDBACK));
					MessageBox(GetHwnd(), msg.c_str(), strTable.Load(IDS_INSTALLCANCELLEDTITLE), MB_OK | MB_ICONINFORMATION);
				}
				else
				{
					msg.append(strTable.LoadS(IDS_INSTALLPARTIAL));
					MessageBox(GetHwnd(), msg.c_str(), strTable.Load(IDS_INSTALLCANCELLEDTITLE), MB_OK | MB_ICONWARNING);
				}
				CloseDlg();
			}
			else
			{
				const InstallerException *pEx = installManager->GetInstallerException();
				std::wstring msg;
				if (pEx != nullptr)
				{
					const InstallerErrorInformation& inf = pEx->GetErrorInfo();
					boost::filesystem::path pkgPath;
					if (inf.currentPackage.packageName.length() > 0)
					{
						pkgPath = inf.currentPackage.parentPackageName.length() > 0 ? 
							inf.currentPackage.parentPackageName : pEx->GetErrorInfo().currentPackage.packageName;
						msg.append(pkgPath.filename().wstring());
						msg.append(L": ");

						// Lösche das Package welches den Fehler verursacht hat
						selectedPackages.erase(remove_if(begin(selectedPackages), end(selectedPackages), 
							[&pkgPath](const PkgDispInfo& disp) { return disp.completePath == pkgPath.wstring(); } ), end(selectedPackages));
					}
					
					switch (inf.kindOfError)
					{
					case InstallerErrorInformation::InstallErrorFileSystemOccurred:
						msg.append(strTable.LoadS(IDS_INSTALLERRORFS));
						break;
					case InstallerErrorInformation::InstallErrorTxFsOccurred:
						msg.append(strTable.LoadS(IDS_INSTALLERRORTXFS));
						break;
					case InstallerErrorInformation::InstallErrorZipOccurred:
						msg.append(strTable.LoadS(IDS_INSTALLERRORZIP));
						break;
					case InstallerErrorInformation::InstallErrorPkgVersionNewer:
						msg.append(strTable.LoadS(IDS_PKGVERSIONNEWER));
						break;
					case InstallerErrorInformation::InstallErrorDeinstall:
						msg.append(strTable.LoadS(IDS_DEINSTALLERRORUNKWOWN));
						break;
					}
					if (!inf.currentFile.empty())
					{
						msg.append(L"\n").append(inf.currentFile).append(L"\n");
					}
					if (inf.errorMsg.length() > 0)
					{
						msg.append(inf.errorMsg).append(L"\n");
					}
				}	
				else
				{
					msg = strTable.LoadS(IDS_INSTALLERROR);
				}
				if (installManager->IsUsingTransaction())
				{
					msg.append(strTable.LoadS(IDS_INSTALLROLLEDBACK));
				}
				else
				{
					msg.append(strTable.LoadS(IDS_INSTALLPARTIAL));
				}
				MessageBox(GetHwnd(), msg.c_str(), strTable.Load(IDS_INSTALLERRORHEADER), MB_OK | MB_ICONERROR);
				CloseDlg(true);
			}
		}
	}
	return Dialog::OnTimer(wParam, lParam);
}

}
}
}