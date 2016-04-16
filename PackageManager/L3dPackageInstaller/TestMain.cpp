#include "stdafx.h"
#include "TestMain.h"

#include <string>
#include <unordered_map>

#include <boost/algorithm/string.hpp> 

#include <lhException.h>
#include <lhWinAPI.h>

#include "DBHelper.h"
#include "InstallManager.h"


namespace l3d
{
namespace packageinstaller
{
namespace tests
{

TestMain::TestMain(const boost::filesystem::path& l3dPath) :
	datadirPath_(l3dPath / L"unittest_datadir"),
	packagesPath_(boost::filesystem::path(__FILE__).parent_path().parent_path() / L"unittest_packages")
{
}

void TestMain::RunTests()
{
	PrepareTest();
	Test1();
	TeardownTest();

	PrepareTest();
	Test2();
	TeardownTest();

	PrepareTest();
	Test3();
	TeardownTest();
}


void TestMain::Log(const std::wstring& msg)
{
}

void TestMain::PrepareTest()
{
	boost::filesystem::remove_all(datadirPath_);
	boost::filesystem::create_directory(datadirPath_);

	db::DBHelper::instance().InitDb((datadirPath_ / L"l3dpackagemanager.sqlite").wstring(), datadirPath_);
}

void TestMain::TeardownTest()
{
	db::DBHelper::instance().Close();
}

void TestMain::Test1()
{
	{
		InstallManager instMgmt(datadirPath_.wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg1.l3dpack").wstring());
		instMgmt.SetReplaceOnlyOlder(true);
		instMgmt.SetAlwaysAskBeforeOverwrite(false);
		instMgmt.StartInstallation();
		instMgmt.WaitUntilFinished();
	}

	{
		InstallManager instMgmt(datadirPath_.wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg2.l3dpack").wstring());
		instMgmt.SetReplaceOnlyOlder(true);
		instMgmt.SetAlwaysAskBeforeOverwrite(false);
		instMgmt.StartInstallation();
		instMgmt.WaitUntilFinished();
	}

	std::unordered_map<std::wstring, std::wstring> expectedContent;
	expectedContent[L"fstnewer-a2.txt"] = L"A2-new";
	expectedContent[L"fstnewer-a3.txt"] = L"A3-new";
	expectedContent[L"fstolder-a1.txt"] = L"A1-new";
	expectedContent[L"fstolder-a4.txt"] = L"A4-new";
	expectedContent[L"sndpackage-a5.txt"] = L"A5";

	CheckInstallation(expectedContent);

	{
		InstallManager instMgmt(datadirPath_.wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg3.l3dpack").wstring());
		instMgmt.SetReplaceOnlyOlder(true);
		instMgmt.SetAlwaysAskBeforeOverwrite(false);
		instMgmt.StartInstallation();
		instMgmt.WaitUntilFinished();
	}

	expectedContent[L"fstnewer-a2.txt"] = L"A2-new";
	expectedContent[L"fstnewer-a3.txt"] = L"A3-new";
	expectedContent[L"fstolder-a1.txt"] = L"A1-new";
	expectedContent[L"fstolder-a4.txt"] = L"A4-new";
	expectedContent[L"sndpackage-a5.txt"] = L"A5";
	expectedContent[L"fstpackage-a6.txt"] = L"A6";
	expectedContent[L"trdpackage-a7.txt"] = L"A7";

	CheckInstallation(expectedContent);
}

void TestMain::Test2()
{
	{
		InstallManager instMgmt(datadirPath_.wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg1.l3dpack").wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg2.l3dpack").wstring());
		instMgmt.SetReplaceOnlyOlder(true);
		instMgmt.SetAlwaysAskBeforeOverwrite(false);
		instMgmt.StartInstallation();
		instMgmt.WaitUntilFinished();
	}

	std::unordered_map<std::wstring, std::wstring> expectedContent;
	expectedContent[L"fstnewer-a2.txt"] = L"A2";
	expectedContent[L"fstnewer-a3.txt"] = L"A3";
	expectedContent[L"fstolder-a1.txt"] = L"A1-new";
	expectedContent[L"fstolder-a4.txt"] = L"A4-new";
	expectedContent[L"sndpackage-a5.txt"] = L"A5";

	CheckInstallation(expectedContent);
}

void TestMain::Test3()
{
	{
		InstallManager instMgmt(datadirPath_.wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg1.l3dpack").wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg2.l3dpack").wstring());
		instMgmt.AddPackageToInstall((packagesPath_ / L"deinstallation-installation-pkg3.l3dpack").wstring());
		instMgmt.SetReplaceOnlyOlder(true);
		instMgmt.SetAlwaysAskBeforeOverwrite(false);
		instMgmt.StartInstallation();
		instMgmt.WaitUntilFinished();
	}

	std::unordered_map<std::wstring, std::wstring> expectedContent;
	expectedContent[L"fstnewer-a2.txt"] = L"A2-new";
	expectedContent[L"fstnewer-a3.txt"] = L"A3";
	expectedContent[L"fstolder-a1.txt"] = L"A1-new";
	expectedContent[L"fstolder-a4.txt"] = L"A4-new";
	expectedContent[L"sndpackage-a5.txt"] = L"A5";
	expectedContent[L"fstpackage-a6.txt"] = L"A6";
	expectedContent[L"trdpackage-a7.txt"] = L"A7";

	CheckInstallation(expectedContent);
}

void TestMain::CheckInstallation(std::unordered_map<std::wstring, std::wstring>& expectedContent)
{
	boost::filesystem::directory_iterator itEnd;
	for (boost::filesystem::directory_iterator itFile(datadirPath_ / L"install-test1"); itFile != itEnd; ++itFile) {
		lhWinAPI::TextFile tf(itFile->path().wstring(), GENERIC_READ, 0, OPEN_EXISTING);
		std::wstring txt;
		tf.ReadText(txt, lhWinAPI::TextFile::UTF8);
		std::wstringstream txtStream(txt);
		std::wstring fstLine;
		std::getline(txtStream, fstLine);

		bool foundFile = false;
		for (auto itMap = expectedContent.begin(); itMap != expectedContent.end(); ++itMap) {
			if (boost::iequals(itFile->path().filename().wstring(), itMap->first)) {
				foundFile = true;
				if (boost::iequals(fstLine, itMap->second)) {
					expectedContent.erase(itMap);
					break;
				}
				else {
					throw lhstd::exception(L"wrong version of " + itMap->first + L"\n" +
										   L"Content was: " + fstLine + L"\nExpected: " + itMap->second);
				}
			}
		}

		if (!foundFile) {
			throw lhstd::exception(L"unknown file " + itFile->path().filename().wstring());
		}
	}
	if (!expectedContent.empty()) {
		throw lhstd::exception(L"map not empty - " + expectedContent.begin()->first);
	}
}

}
}
}