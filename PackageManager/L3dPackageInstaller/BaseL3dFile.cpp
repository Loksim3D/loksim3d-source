#include "stdafx.h"

#include "BaseL3dFile.h"

#include <chrono>
#include <sstream>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "pugixml/pugixml.hpp"

#include "FileDescriptions.h"
#include "XmlHelper.h"

namespace l3d
{
namespace files
{

using namespace std;

/**
* Lädt die Basisdaten die bei allen Dateien gleich sind
*/
void BaseL3dFile::LoadBaseFileData(const common::L3dPath& filePath)
{
	pugi::xml_document d;
	ReadFromFile(filePath, d);
}

void BaseL3dFile::ReadFromFile(const common::L3dPath& filePath, pugi::xml_document& xmlDoc)
{
	helper::ReadFromFile(filePath, xmlDoc);
	filePath_ = filePath;

	pugi::xml_node propsNode = xmlDoc.first_child().child(FILE_GENERAL_PROPS);
	if (propsNode) {
		fileAuthors_ = helper::ReadStringVector(propsNode, FILE_GENERAL_AUTHOR);
		fileInfo_ = propsNode.attribute(FILE_GENERAL_INFO).value();
		filePicture_ = propsNode.attribute(FILE_GENERAL_PICTURE).value();
		fileDoc_ = propsNode.attribute(FILE_GENERAL_DOC).value();
		fileEditorVersion_ = propsNode.attribute(FILE_GENERAL_EDITOR_VERSION).as_int();
		//if (fileEditorVersion_ > L3D_VERSION_CODE) {
		//	LOG_INFO << L"File was created with newer Loksim Version " << fileEditorVersion_ << L" > " << L3D_VERSION_CODE
		//		<< ", File: " << filePath.GetL3dDirRelativePath();
		//}
	}
}

void BaseL3dFile::WriteToXmlNode(pugi::xml_node& nRoot)
{
	WriteToXmlNode(nRoot, filePath_);
}

void BaseL3dFile::WriteToXmlNode(pugi::xml_node& nRoot, const common::L3dPath& filePath)
{
	assert(false);
}

bool BaseL3dFile::WriteToFile(const common::L3dPath& filePath)
{
	pugi::xml_document doc;
	pugi::xml_node nRoot = doc.append_child(GetRootNodeName().c_str());

	WriteToXmlNode(nRoot, filePath);

	if (helper::SaveToFile(doc, filePath)) {
		filePath_ = filePath;
		return true;
	}
	return false;
}

const common::L3dPath& BaseL3dFile::GetFilePath() const
{
	return filePath_;
}

const std::wstring& BaseL3dFile::GetFileInfo() const
{
	return fileInfo_;
}
void BaseL3dFile::SetFileInfo(const std::wstring& fileInfo)
{
	fileInfo_ = fileInfo;
}

const std::vector<std::wstring>& BaseL3dFile::GetFileAuthors() const
{
	return fileAuthors_;
}

std::wstring BaseL3dFile::GetFileAuthorsStr() const
{
	if (fileAuthors_.empty()) {
		return L"";
	}
	else {
		std::wstringstream str;
		for (const auto& el : fileAuthors_) {
			str << el;
			str << L"; ";
		}
		auto s = str.str();
		return s.substr(0, s.size() - 2);
	}
}

void BaseL3dFile::SetFileAuthors(std::vector<std::wstring> fileAuthors)
{
	fileAuthors_ = std::move(fileAuthors);
}

void BaseL3dFile::SetFileAuthors(const std::wstring& fileAuthors)
{
	fileAuthors_.clear();
	if (!fileAuthors.empty()) {
		boost::split(fileAuthors_, fileAuthors, [](wchar_t t) { return t == ';'; }, boost::algorithm::token_compress_on);
		for (auto& a : fileAuthors_) {
			boost::trim(a);
		}
	}
}

bool BaseL3dFile::IsAuthor(std::wstring author)
{
	boost::trim(author);
	for (auto& a : fileAuthors_) {
		if (boost::algorithm::iequals(a, author)) {
			return true;
		}
	}
	return false;
}

const wstring& BaseL3dFile::GetFilePicture() const
{
	return filePicture_;
}
void BaseL3dFile::SetFilePicture(const std::wstring& filePicture)
{
	filePicture_ = filePicture;
}

const wstring& BaseL3dFile::GetFileDoc() const
{
	return fileDoc_;
}
void BaseL3dFile::SetFileDoc(const std::wstring& fileDoc)
{
	fileDoc_ = fileDoc;
}

int BaseL3dFile::GetFileEditorVersion() const
{
	return fileEditorVersion_;
}

void BaseL3dFile::SetFilePath(common::L3dPath filePath)
{
	filePath_ = std::move(filePath);
}

}
}