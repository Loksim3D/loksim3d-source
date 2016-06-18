#pragma once

#include <string>
#include <vector>

#include "BaseL3dFile.h"

namespace l3d
{

/**
* Klasse welche die Beschreibung eines Loksim3D - Package kapselt
*/
class L3dPackageInfo :
	public files::BaseL3dFile
{
public:
	L3dPackageInfo(void);
	~L3dPackageInfo(void);

	bool LoadFromFile(const std::wstring& filePath);

	virtual void SaveDataToXmlNode(const l3d::common::L3dPath&, pugi::xml_node&) const override;
	virtual std::wstring GetRootNodeName() const override;

	inline const std::vector<std::wstring>&	GetFilesToDelete() const { return filesToDelete_; }
	inline const std::vector<std::wstring>&	GetPkgsToDeinstall() const { return pkgsToDeinstall_; }
	inline const std::vector<std::wstring>&	GetAddonIdsToDeinstall() const { return addonIdsToDeinstall_; }
	inline const std::wstring& GetDisplayName() const { return displayName_; }
	inline const std::wstring& GetAddonID() const { return addonId_; }
	inline const int GetAddonVersion() const { return addonVersion_; }
	inline int GetMinimumLoksimVersion() const { return GetFileEditorVersion(); }

	void AddFileToDelete(const std::wstring& file) { filesToDelete_.push_back(file); }
	void AddPkgHashToDeinstall(const std::wstring& hash) { pkgsToDeinstall_.push_back(hash); }
	void AddAddonIdToDeinstall(const std::wstring& hash) { addonIdsToDeinstall_.push_back(hash); }
	inline void SetDisplayName(const std::wstring& name) { displayName_ = name; }
	inline void SetAddonID(const std::wstring& id) { addonId_ = id; }
	inline void SetAddonVersion(int addonVersion) { addonVersion_ = addonVersion; }

private:
	void Clear();

	std::vector<std::wstring> filesToDelete_;
	std::vector<std::wstring> pkgsToDeinstall_; // Identified by Hash
	std::vector<std::wstring> addonIdsToDeinstall_; // Identified by User-chosen ID
	std::wstring displayName_;
	std::wstring addonId_;
	int addonVersion_;
};


}