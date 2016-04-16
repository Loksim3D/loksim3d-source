// L3dPackageInstaller.cpp : Defines the entry point for the application.
//

//#pragma comment(linker,"\"/manifestdependency:type='win32' \
//name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
//processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


#include "stdafx.h"
#include "L3dPackageInstaller.h"

#include <CrashRpt.h>
#include <CrashRptSettings.h>

#include <Ole2.h>
#include <Commctrl.h>
#include <ShellAPI.h>
#include "DlgPackageInstall.h"
#include "DlgDeinstallSelectPkg.h"
#include "DBHelper.h"
#include "MainDlg.h"
#include "TestMain.h"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp> 
#include <boost/date_time/posix_time/posix_time.hpp>

#include <memory>
#include <string>

#include <pathsini.h>

#include "RegistryAccess.h"

#include "FileSystemUtils.h"

#include "KTM.h"


using namespace l3d::packageinstaller;
using namespace l3d::packageinstaller::ui;
using namespace std;

#define MAX_LOADSTRING 100


void SetLanguage()
{
	wstring cul = RegistryAccess::GetUiCulture();
	if (cul.length() > 0)
	{
		int langId;
		if (boost::algorithm::iequals(cul, L"de-DE"))
		{
			langId = 0x0407;
			//SetThreadUILanguage(0x0407/*MAKELANGID (LANG_GERMAN, SUBLANG_NEUTRAL)*/);	//0x0407 = German (Germany)
		}
		else /*if (boost::algorithm::iequals(cul, L"en-US"))*/
		{
			langId = 0x0409;
			//SetThreadUILanguage(0x0409/*MAKELANGID (LANG_ENGLISH, SUBLANG_NEUTRAL)*/);	//0x0409 = English (US)
		}

		//Vista oder höher
		SetThreadUILanguage(langId);
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	CR_INSTALL_INFO cri = { 0 };
	TCHAR szFilename[MAX_PATH + 1];
	szFilename[0] = '\0';
	wstring fileVers;
	
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) != 0)
	{
		lhWinAPI::GetFileVersion(wstring(szFilename), fileVers, nullptr);
	}

	cri.cb = sizeof(CR_INSTALL_INFO);    
	cri.pszAppName = L"PackageManager";
	cri.pszAppVersion = fileVers.c_str();
	//cri.pszEmailSubject = L"PackageManager Error Report";
	//cri.pszEmailTo = l3d::crashreport::CRASH_REPORT_MAIL;
	cri.pszUrl = l3d::crashreport::CRASH_REPORT_URL;
	cri.pszPrivacyPolicyURL = l3d::crashreport::CRASH_REPORT_PRIVACY_POLICY_URL;
	cri.uPriorities[CR_HTTP] = 3;  // First try send report over HTTP 
	cri.uPriorities[CR_SMAPI] = CR_NEGATIVE_PRIORITY; // Second try send report over Simple MAPI    
	cri.uPriorities[CR_SMTP] = CR_NEGATIVE_PRIORITY;
	// Install all available exception handlers, use HTTP binary transfer encoding (recommended).
	cri.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
	//cri.dwFlags |= CR_INST_HTTP_BINARY_ENCODING; 
	cri.dwFlags |= CR_INST_APP_RESTART; 
	cri.dwFlags |= CR_INST_SEND_QUEUED_REPORTS; 
	cri.dwFlags |= CR_INST_ALLOW_ATTACH_MORE_FILES;
	cri.dwFlags |= CR_INST_AUTO_THREAD_HANDLERS;
	cri.dwFlags |= CR_INST_SHOW_ADDITIONAL_INFO_FIELDS;


	// Language File für CrashRpt abhängig von eingestellter Sprache
	auto lang = l3d::packageinstaller::RegistryAccess::GetUiCulture();
	if (boost::istarts_with(lang, L"en"))
	{
		lang = lhWinAPI::FilePathExec(L"Language Files\\crashrpt_lang_EN.ini");
		cri.pszLangFilePath = lang.c_str();
	}
	else
	{
		lang = lhWinAPI::FilePathExec(L"Language Files\\crashrpt_lang_DE.ini");
		cri.pszLangFilePath = lang.c_str();
	}

	CrAutoInstallHelper cr_install_helper(&cri);

	//wchar_t buf[1024];
	//crGetLastErrorMsg(buf, 1024);

	crAddRegKey(_T("HKEY_CURRENT_USER\\Software\\Loksim-Group"), _T("regkey.xml"), 0);

	if (szFilename[0] != '\0')
	{
		auto t = boost::filesystem::last_write_time(szFilename);
		auto posixTime = boost::posix_time::from_time_t(t);
		crAddProperty(L"ExeLastWrite", boost::posix_time::to_simple_wstring(posixTime).c_str());
	}

	crAddFile2(l3d::packageinstaller::fs::GetFilenameInLocalSettingsDir(
		l3d::packageinstaller::fs::FILENAME_CRASHRPT_LOG).c_str(), L"LetzteInstallation.txt", 
		L"Letzte Installation", CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE);

	//crAddScreenshot2(CR_AS_PROCESS_WINDOWS, 100);


	SetLanguage();

	boost::filesystem::path l3dPath = lhWinAPI::FilePathExec(L"Loksim3D.exe");
	l3dPath = l3dPath.parent_path();

	crAddFile2((l3dPath / L"paths.ini").c_str(), L"paths.ini", L"Loksim Pfade", CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE);

	l3dPath = l3d::config::GetDataDirPathFromPathsIni(l3dPath);

#ifdef _DEBUG
	//l3dPath = L"D:\\LoksimDevelopment\\RainerDir";
#endif

	crAddFile2((l3dPath / L"l3dpackagemanager.sqlite").c_str(), L"l3dpackagemanager.sqlite", L"Package Datenbank", CR_AF_MISSING_FILE_OK | CR_AF_ALLOW_DELETE);

	// Testen ob Package-Manager ins Verzeichnis schreiben kann
	wchar_t tempOut[MAX_PATH];
	if (0 == GetTempFileName(l3dPath.c_str(), L"pkg", 0, tempOut))
	{
		if (GetLastError() == ERROR_ACCESS_DENIED)
		{
			HANDLE hToken;
			OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken);
			TOKEN_ELEVATION elevation;
			DWORD infoLen;
			GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &infoLen);
			if (!elevation.TokenIsElevated)
			{
				// Wir sind nicht "elevated" => runas ausführen damit wir admin rechte zum schreiben bekommen
				GetModuleFileName(nullptr, tempOut, MAX_PATH);
				ShellExecute(nullptr,  
					L"runas",   
					tempOut, 
					lpCmdLine,      
					nullptr,   
					SW_SHOWNORMAL); 
			}
			else
			{
				lhWinAPI::StringTable st;
				TCHAR temp[MAX_PATH + 128];
				swprintf_s(temp, st.Load(IDS_ERRORACCESS), l3dPath.c_str());
				MessageBox(nullptr, temp, nullptr, MB_OK | MB_ICONERROR);
			}
			return 0;
		}
	}
	else
	{
		DeleteFile(tempOut);
	}

	InitCommonControls();

	//l3dPath /= L"\\";

	// "Testfälle" für Absolut in Relativ umwandlung
	//auto r1 = l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(L"D:\\LoksimDevelopment\\Loksim-Data\\Objekte", l3dPath);
	//auto r2 = l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(L"D:\\LoksimDevelopment\\Loksim-Data\\Objekte\\", l3dPath);
	//auto r3 = l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(L"D:\\LoksimDevelopment/Loksim-Data\\Objekte\\test.l3dobj", l3dPath);

	//auto r4 = l3d::packageinstaller::fs::AbsoluteToLoksimRelativePath(L"D:\\LoksimDevelopment\\Loksim-Data2\\Objekte\\test.l3dobj", l3dPath);

	//auto a1 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"Objekte\\", l3dPath);
	//auto a2 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"Objekte", l3dPath);
	//auto a3 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"\\Objekte\\", l3dPath);
	//auto a4 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"\\Objekte", l3dPath);

	//auto a5 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"\\Objekte\\test.l3dobj", l3dPath);
	//auto a6 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"Objekte\\test.l3dobj", l3dPath);

	//auto a7 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"X:\\LoksimDevelopment\\Loksim-Data\\Objekte", l3dPath);
	//auto a8 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"X:\\LoksimDevelopment\\Loksim-Data\\Objekte\\", l3dPath);
	//auto a9 = l3d::packageinstaller::fs::LoksimRelativeToAbsolutePath(L"X:\\LoksimDevelopment\\Loksim-Data\\Objekte\\test.l3dobj", l3dPath);

	//boost::filesystem::path p = "D:\\LoksimDevelopment\\RainerDir\\Signale/Vorsignale/Bake 300m.l3dgrp.jpg";
	//auto t1 = p.parent_path();
	//p = "D:\\LoksimDevelopment\\RainerDir\\Signale\\Vorsignale\\Bake 300m.l3dgrp.jpg";
	//auto t2 = p.parent_path();
	//p = "D:\\LoksimDevelopment\\RainerDir\\Signale\\Vorsignale\\Bake 300m.l3dgrp.jpg";
	//auto t3 = p.parent_path();


	KTMTransaction::TestIfTxFsAvailableInDir(l3dPath.c_str());

	try
	{
		hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_L3DPACKAGEINSTALLER));

		OleInitialize(nullptr);

		{
			wofstream outStr(l3d::packageinstaller::fs::GetFilenameInLocalSettingsDir(l3d::packageinstaller::fs::FILENAME_CRASHRPT_LOG));
			outStr << L"l3dPath: " << l3dPath << endl;
		}

		if (!boost::filesystem::exists(l3dPath))
		{
			lhWinAPI::StringTable st;
			TCHAR temp[1024];
			swprintf_s(temp, st.Load(IDS_ERR_PATH_DOES_NOT_EXIST), l3dPath.c_str());
			MessageBox(nullptr, temp, nullptr, MB_OK | MB_ICONERROR);
			return 1;
		}

		std::vector<std::wstring> args;
		for (int i = 1; i < __argc; ++i)
		{
			args.emplace_back(__wargv[i]);
		}

		if (!args.empty() && boost::iequals(args[0], L"/unittests")) {
			l3d::packageinstaller::tests::TestMain tests(l3dPath);
			tests.RunTests();
			return 0;
		}

		db::DBHelper::instance().InitDb((l3dPath / L"l3dpackagemanager.sqlite").wstring(), l3dPath);

		if (args.size() >= 2)
		{
			if (boost::iequals(args[0], L"/q"))
			{
				{
					InstallManager instMgmt(l3dPath.wstring());
					for (size_t i = 1; i < args.size(); i++)
					{
						instMgmt.AddPackageToInstall(args[i]);
					}
					instMgmt.SetReplaceOnlyOlder(true);
					instMgmt.SetAlwaysAskBeforeOverwrite(false);
					instMgmt.StartInstallation();
					instMgmt.WaitUntilFinished();
				}
				return 0;
			}
		}

		MainDlg dlg(l3dPath.wstring(), args);
		dlg.Show(hInstance, nullptr);

		// Main message loop:
		while (GetMessage(&msg, NULL, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		OleUninitialize();
	}
	catch (const Kompex::SQLiteException& ex)
	{
		lhWinAPI::StringTable st;
		OutputDebugStringA(ex.GetString().c_str());
		std::wstring strMsg;
		lhWinAPI::CharToTchar(strMsg, ex.GetString());
		TCHAR temp[1024];
		swprintf_s(temp, st.Load(IDS_ERRORDB), strMsg.c_str());

		MessageBox(nullptr, temp, nullptr, MB_OK | MB_ICONERROR);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		lhWinAPI::StringTable st;
		OutputDebugStringA(ex.what());
		std::wstring strMsg;
		lhWinAPI::CharToTchar(strMsg, ex.what());
		TCHAR temp[1024];
		swprintf_s(temp, st.Load(IDS_ERR_CREATE_DIR), l3dPath.c_str(), strMsg.c_str());
		MessageBox(nullptr, temp, nullptr, MB_OK | MB_ICONERROR);
	}

	try
	{
		boost::filesystem::remove(l3d::packageinstaller::fs::GetFilenameInLocalSettingsDir(
			l3d::packageinstaller::fs::FILENAME_CRASHRPT_LOG));
	} catch(...)
	{
	}

	return (int) msg.wParam;
}


