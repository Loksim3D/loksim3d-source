#pragma once

#include "pugixml/pugixml.hpp"

#include <string>



namespace l3d
{

/**
 * Basisklasse für alle Loksim3D XMLbasierten-Dateien
 */
class BaseL3dFile
{
public:
	BaseL3dFile(void);
	~BaseL3dFile(void);
	
	/**
	 * Liefert Pfad dieser L3D-Datei
	 */
	const std::wstring& GetFilePath() const;
	
	/**
	 * Liefert Datei-Beschreibung
	 */
	const std::wstring& GetFileInfo() const;
	void SetFileInfo(const std::wstring& fileInfo);

	/**
	 * Liefert Datei Autor
	 */
	const std::wstring& GetFileAuthor() const;
	void SetFileAuthor(const std::wstring& fileAuthor);

	/**
	 * Liefert Datei Bild
	 */
	const std::wstring& GetFilePicture() const;
	void SetFilePicture(const std::wstring& filePicture);


	bool WriteToFile(const std::wstring& filePath);

protected:
	/**
	 * Lädt den gesamten Inhalt der Datei in den Speicher in xmlDoc<br>
	 */
	bool ReadFromFile(const std::wstring& filePath, pugi::xml_document& xmlDoc);

	/**
	 * Speichert die Daten 
	 */
	virtual void SaveDataToDoc(pugi::xml_document& targetDoc) const = 0;

	virtual std::wstring GetRootNodeName() const = 0;

private:
	std::wstring _filePath;
	std::wstring _fileInfo;
	std::wstring _fileAuthor;
	std::wstring _filePicture;

};


}