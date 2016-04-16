#pragma once

#include <string>
#include <unordered_map>

#include <boost/filesystem.hpp>

namespace l3d
{
namespace packageinstaller
{
namespace tests
{

class TestMain
{
public:
	TestMain(const boost::filesystem::path& l3dPath);

	void RunTests();

private:
	void Log(const std::wstring& msg);
	void PrepareTest();
	void TeardownTest();

	void Test1();
	void Test2();
	void Test3();

	void CheckInstallation(std::unordered_map<std::wstring, std::wstring>& expectedContent);

	boost::filesystem::path datadirPath_;
	boost::filesystem::path packagesPath_;
};

}
}
}

