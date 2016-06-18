#include "stdafx.h"
#include "L3dPackageInfo.h"

#include <algorithm>

#include "pugixml/pugixml.hpp"

#include "L3dConsts.h"

namespace l3d
{

using namespace std;

L3dPackageInfo::L3dPackageInfo(void)
{
}

L3dPackageInfo::~L3dPackageInfo(void)
{
}


bool L3dPackageInfo::LoadFromFile(const std::wstring& filePath)
{
	pugi::xml_document xmlDoc;
	
	ReadFromFile(filePath, xmlDoc);

	{
		pugi::xml_node rootNode = xmlDoc.child(cPKGINFORoot);

		if (rootNode)
		{
			Clear();

			pugi::xml_node propsNode = rootNode.child(cFileProps);
			if (propsNode) {
				displayName_ = propsNode.attribute(cPKGINFODisplayName).value();
				addonId_ = propsNode.attribute(cPKGINFOAddonID).value();
				addonVersion_ = propsNode.attribute(cPKGINFOAddonVersion).as_int();
			}

			pugi::xml_node versNode = rootNode.child(cPKGINFOVersionInfo);
			if (versNode && this->GetFileEditorVersion() == 0)
			{
				fileEditorVersion_ = versNode.attribute(cPKGINFOVersionInfoCode).as_int();
			}
			auto delFilesNode = rootNode.child(cPKGINFODeleteFilesNode);
			if (delFilesNode)
			{
				for (pugi::xml_node n = delFilesNode.child(cPKGINFODeleteFileNode); n; n = n.next_sibling(cPKGINFODeleteFileNode))
				{
					auto attr = n.attribute(cPKGINFODeleteFileName);
					if (attr)
					{
						filesToDelete_.emplace_back(attr.value());
					}
				}
			}

			auto delPkgsNode = rootNode.child(cPKGINFODeinstallPackages);
			if (delPkgsNode)
			{
				for (pugi::xml_node n = delPkgsNode.child(cPKGINFODeinstallPackageNode); n; n = n.next_sibling(cPKGINFODeinstallPackageNode))
				{
					auto attr = n.attribute(cPKGINFODeinstallPackageHash);
					if (attr)
					{
						pkgsToDeinstall_.emplace_back(attr.value());
					}

					attr = n.attribute(cPKGINFODeinstallAddonID);
					if (attr) {
						addonIdsToDeinstall_.emplace_back(attr.value());
					}
				}
			}
			return true;
		}
	}
	return false;
}


//void L3dPackageInfo::SaveDataToDoc(pugi::xml_document& targetDoc) const
//{
//	assert(false);
//}
//
//void files::L3dPackageInfo::SaveDataToXmlNode(const l3d::common::L3dPath &, pugi::xml_node &) const
//{
//	assert(false);
//}

void L3dPackageInfo::SaveDataToXmlNode(const l3d::common::L3dPath&, pugi::xml_node&) const
{
	assert(false);
}

std::wstring L3dPackageInfo::GetRootNodeName() const
{
	return cPKGINFORoot;
}

void L3dPackageInfo::Clear()
{
	filesToDelete_.clear();
	pkgsToDeinstall_.clear(); // Identified by Hash
	addonIdsToDeinstall_.clear(); // Identified by User-chosen ID
	displayName_.clear();
	addonId_.clear();
	addonVersion_ = 0;
}

}