#include "stdafx.h"
#include "BaseL3dFile.h"

#include "L3dConsts.h"

namespace l3d
{

using namespace std;

BaseL3dFile::BaseL3dFile(void)
{
}


BaseL3dFile::~BaseL3dFile(void)
{
}

bool BaseL3dFile::ReadFromFile(const wstring& filePath, pugi::xml_document& xmlDoc)
{
	_filePath = filePath;

	pugi::xml_parse_result result = xmlDoc.load_file(filePath.c_str());

	if (result.status != pugi::status_ok)
	{
		return false;
	} 
	else 
	{
		pugi::xml_node propsNode = xmlDoc.first_child().child(cFileProps);
		if (propsNode)
		{
			_fileAuthor = propsNode.attribute(cFileAutor).value();
			_fileInfo = propsNode.attribute(cFileInfo).value();
			_filePicture = propsNode.attribute(cFilePicture).value();
		}
		return true;
	}
}

bool BaseL3dFile::WriteToFile(const std::wstring& filePath)
{
	pugi::xml_document doc;
	pugi::xml_node nRoot = doc.append_child(GetRootNodeName().c_str());
	pugi::xml_node nProps = nRoot.append_child(cFileProps);
	nProps.append_attribute(cFileAutor).set_value(_fileAuthor.c_str());
	nProps.append_attribute(cFileInfo).set_value(_fileInfo.c_str());
	nProps.append_attribute(cFilePicture).set_value(_filePicture.c_str());

	SaveDataToDoc(doc);
	bool r = doc.save_file(filePath.c_str());
	if (r)
	{
		_filePath = filePath;
	}
	return r;
}


/*pugi::xml_document&  BaseL3dFile::GetXmlDoc()
{
	return xmlDoc;
}
*/


const wstring& BaseL3dFile::GetFilePath() const
{
	return _filePath;
}

const std::wstring& BaseL3dFile::GetFileInfo() const
{
	return _fileInfo;
}
void BaseL3dFile::SetFileInfo(const std::wstring& fileInfo)
{
	_fileInfo = fileInfo;
}

const std::wstring& BaseL3dFile::GetFileAuthor() const
{
	return _fileAuthor;
}
void BaseL3dFile::SetFileAuthor(const std::wstring& fileAuthor)
{
	_fileAuthor = fileAuthor;
}

const wstring& BaseL3dFile::GetFilePicture() const
{
	return _filePicture;
}
void BaseL3dFile::SetFilePicture(const std::wstring& filePicture)
{
	_filePicture = filePicture;
}

}