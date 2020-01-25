#include "stdafx.h"

#include "L3dPath.h"

#include <boost/algorithm/string.hpp>
#include <boost/version.hpp> 

namespace l3d
{
namespace common
{

using namespace boost::filesystem;
using namespace std;


L3dPath L3dPath::loksimDir_;

L3dPath::L3dPath(const path& absolutePath) : absolutePath_(absolutePath)
{
	NormalizePath();
}

L3dPath::L3dPath(path&& absolutePath) : absolutePath_(move(absolutePath))
{
	NormalizePath();
}

L3dPath::L3dPath(const wchar_t* absolutePath) : absolutePath_(absolutePath)
{
	NormalizePath();
}

L3dPath::L3dPath(std::wstring absolutePath) : absolutePath_(move(absolutePath))
{
	NormalizePath();
}

L3dPath::L3dPath(const char* absolutePath) : absolutePath_(absolutePath)
{
	NormalizePath();
}

L3dPath::L3dPath(std::string absolutePath) : absolutePath_(move(absolutePath))
{
	NormalizePath();
}

template <class T>
L3dPath CreateRelativeToL3dDirIntern(const std::basic_string<T>& relativePath)
{
	assert(!L3dPath::GetLoksimDir().IsEmpty() && "Es muss zuerst das Loksim Verzeichnis gesetzt werden");

	path relPath = relativePath;
	if (relPath.is_absolute()) {
		return L3dPath(move(relPath));
	}
	else {
		path ret = L3dPath::GetLoksimDir().GetAbsolutePath();
		if (relativePath[0] == '/' || relativePath[0] == '\\') {
			ret /= relativePath.substr(1);
		} 
		else {
			ret /= relativePath;
		}
		return L3dPath(move(ret));
	}
}

template <class T>
L3dPath CreateRelativeToFileIntern(const std::basic_string<T>& relativePath, const L3dPath& parentFile)
{
	assert(!L3dPath::GetLoksimDir().IsEmpty() && "Es muss zuerst das Loksim Verzeichnis gesetzt werden");

	if (relativePath.empty())
	{
		// leeren Pfad zurückliefern
		return L3dPath();
	}
	if (relativePath[0] == '\\' || relativePath[0] == '/')
	{
		return CreateRelativeToL3dDirIntern(relativePath);
	}

	path relPath = relativePath;
	if (relPath.is_absolute())
	{
		return L3dPath(move(relPath));
	}
	else
	{
		path ret = parentFile.GetAbsolutePath().parent_path();
		ret /= relativePath;
		return L3dPath(move(ret));
	}
}

L3dPath L3dPath::CreateRelativeToL3dDir(const std::wstring& relativePath)
{
	return CreateRelativeToL3dDirIntern(relativePath);
}


L3dPath L3dPath::CreateRelativeToFile(const std::wstring& relativePath, const L3dPath& parentFile)
{
	return CreateRelativeToFileIntern(relativePath, parentFile);
}

L3dPath L3dPath::CreateRelativeToL3dDir(const std::string& relativePath)
{
	return CreateRelativeToL3dDirIntern(relativePath);
}


L3dPath L3dPath::CreateRelativeToFile(const std::string& relativePath, const L3dPath& parentFile)
{
	return CreateRelativeToFileIntern(relativePath, parentFile);
}

std::wstring L3dPath::GetRelativePath(const L3dPath& parentPath) const
{
	if (IsEmpty()) {
		return L"";
	}

	wstring parentDir = parentPath.GetAbsolutePath().parent_path().wstring(); // Ordner der "Elterndatei"
	wstring absDir = absolutePath_.wstring();
	if (!parentPath.IsEmpty() && boost::istarts_with(absDir, parentDir)) {
		return absDir.substr(parentDir.size() + (parentDir.back() == '\\' ? 0 : 1));
    }
	return GetL3dDirRelativePath();
}

std::wstring L3dPath::GetL3dDirRelativePath() const
{
	auto absDir = absolutePath_.native();
	if (boost::istarts_with(absDir, loksimDir_.absolutePath_.native())) {
		return absDir.substr(loksimDir_.absolutePath_.wstring().size() - 1);
    }
	return absDir;
}

bool L3dPath::IsInLoksimDirectory() const
{
	return boost::istarts_with(absolutePath_.native(), loksimDir_.absolutePath_.native());
}

const L3dPath& L3dPath::GetLoksimDir()
{
	return loksimDir_;
}

void L3dPath::SetLoksimDir(const L3dPath& loksimDir) 
{
	//loksimDir_ = loksimDir;

	auto str = loksimDir.absolutePath_.native();
	if (!str.empty() && str.back() == ':') {
		loksimDir_ = loksimDir.GetAbsolutePath().wstring() + boost::filesystem::path::preferred_separator;
	} 
	else {
		if (!str.empty() && str.back() != '\\' && str.back() != '/') {
			loksimDir_ = loksimDir.GetAbsolutePath().native() + L"\\";
		} 
		else {
			loksimDir_ = loksimDir;
		}
	}
}

const boost::filesystem::path& L3dPath::GetAbsolutePath() const
{
	return absolutePath_;
}

bool L3dPath::operator==(const L3dPath& other) const
{
	return GetAbsolutePath() == other.GetAbsolutePath();
}

bool L3dPath::operator!=(const L3dPath& other) const
{
	return GetAbsolutePath() != other.GetAbsolutePath();
}

bool L3dPath::operator<(const L3dPath& other) const
{
	return GetAbsolutePath() < other.GetAbsolutePath();
}

bool L3dPath::IsEmpty() const
{
	return absolutePath_.empty();
}

bool L3dPath::Exists() const
{
	return exists(absolutePath_);
}

void L3dPath::NormalizePath()
{
	if (!absolutePath_.empty()) {
#if BOOST_VERSION >= 106000
		absolutePath_ = absolutePath_.lexically_normal();
		absolutePath_.make_preferred();
		if (!absolutePath_.empty() && absolutePath_.native()[absolutePath_.size() - 1] == '.') {
			absolutePath_ = absolutePath_.native().substr(0, absolutePath_.size() - 1);
		}
#else
		try {
			absolutePath_ = boost::filesystem::canonical(absolutePath_);
		}
		catch (...) {
		}
		absolutePath_.make_preferred();
#endif
	}
}

}
}

