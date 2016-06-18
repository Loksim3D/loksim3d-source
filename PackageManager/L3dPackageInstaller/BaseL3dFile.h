#pragma once

#include <string>


#include "L3dPath.h"

namespace pugi
{
class xml_document;
class xml_node;
}

namespace l3d
{
namespace files
{

/**
* Basisklasse für alle Loksim3D XML-basierten Dateien
*/
class BaseL3dFile
{
public:
	BaseL3dFile(void) = default;
	BaseL3dFile(const BaseL3dFile& other) = default;
	//BaseL3dFile(BaseL3dFile&& other) = default;
	BaseL3dFile& operator=(const BaseL3dFile& other) = default;
	//BaseL3dFile& operator=(BaseL3dFile&& other) = default;
	virtual ~BaseL3dFile(void) = default;

	/**
	* Lädt die Basisdaten die bei allen Dateien gleich sind
	*/
	void LoadBaseFileData(const common::L3dPath& filePath);

	/**
	* Liefert Pfad dieser L3D-Datei
	*/
	const common::L3dPath& GetFilePath() const;
	void SetFilePath(common::L3dPath path);

	/**
	* Liefert Datei-Beschreibung
	*/
	const std::wstring& GetFileInfo() const;
	void SetFileInfo(const std::wstring& fileInfo);

	/**
	* Liefert Datei Autoren
	*/
	const std::vector<std::wstring>& GetFileAuthors() const;
	/**
	* Liefert Dateiautoren als String
	*/
	std::wstring GetFileAuthorsStr() const;
	void SetFileAuthors(std::vector<std::wstring> fileAuthors);
	void SetFileAuthors(const std::wstring& fileAuthors);

	/**
	* Prüft ob der übergebene String (Mit-)Autor der Datei ist
	*/
	bool IsAuthor(std::wstring author);

	/**
	* Liefert Datei Bild
	*/
	const std::wstring& GetFilePicture() const;
	void SetFilePicture(const std::wstring& filePicture);

	/**
	* Liefert Datei Doku
	*/
	const std::wstring& BaseL3dFile::GetFileDoc() const;
	void SetFileDoc(const std::wstring& fileDoc);

	/**
	* Liefert Version des Editors mit welchem Datei zuletzt geändert wurde
	*/
	int GetFileEditorVersion() const;

	bool WriteToFile(const common::L3dPath& filePath);

	void WriteToXmlNode(pugi::xml_node& node);

protected:
	/**
	* Lädt den gesamten Inhalt der Datei in den Speicher in xmlDoc<br>
	*/
	void ReadFromFile(const common::L3dPath& filePath, pugi::xml_document& xmlDoc);

	/**
	* Speichert die Daten
	*/
	virtual void SaveDataToXmlNode(const common::L3dPath&, pugi::xml_node&) const {}

	virtual std::wstring GetRootNodeName() const { return L"Root"; }

	int	fileEditorVersion_;

private:
	void WriteToXmlNode(pugi::xml_node& node, const common::L3dPath& filePath);

	common::L3dPath filePath_;
	std::wstring	fileInfo_;
	std::vector<std::wstring>	fileAuthors_;
	std::wstring	filePicture_;
	std::wstring	fileDoc_;
};

}
}
