#pragma once

#include <string>
#include <vector>

#include "basel3dfile.h"

namespace l3d
{

/**
 * Klasse welche die Beschreibung eines Loksim3D - Package kapselt
 */
class L3dPackageInfo :
	public BaseL3dFile
{
public:
	L3dPackageInfo(void);
	~L3dPackageInfo(void);

	bool LoadFromFile(const std::wstring& filePath);

	virtual void SaveDataToDoc(pugi::xml_document& targetDoc) const;
	virtual std::wstring GetRootNodeName() const;


	inline int GetVersionCode() const { return _versionCode; }
	inline void SetVersionCode(const int versionCode) { _versionCode = versionCode; }

	inline const std::vector<std::wstring>&	GetFilesToDelete() const { return _filesToDelete; }
	inline const std::vector<std::wstring>&	GetPkgsToDeinstall() const { return _pkgsToDeinstall; }

	void AddFileToDelete(const std::wstring& file) { _filesToDelete.push_back(file); }
	void AddPkgHashToDeinstall(const std::wstring& hash) { _pkgsToDeinstall.push_back(hash); }

private:
	int _versionCode;
	std::vector<std::wstring>	_filesToDelete;
	std::vector<std::wstring>	_pkgsToDeinstall;
};

}
