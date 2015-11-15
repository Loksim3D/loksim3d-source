#include "stdafx.h"
#include "L3dPackageInfo.h"
#include "L3dConsts.h"

#include <algorithm>

namespace l3d
{

using namespace std;

L3dPackageInfo::L3dPackageInfo(void) : _versionCode(l3d::LOKSIM3D_VERSION_CODE)
{
}

L3dPackageInfo::~L3dPackageInfo(void)
{
}


bool L3dPackageInfo::LoadFromFile(const std::wstring& filePath)
{
	pugi::xml_document xmlDoc;
	if (ReadFromFile(filePath, xmlDoc))
	{
		pugi::xml_node rootNode = xmlDoc.child(cPKGINFORoot);

		if (rootNode)
		{
			pugi::xml_node versNode = rootNode.child(cPKGINFOVersionInfo);
			if (versNode)
			{
				_versionCode = versNode.attribute(cPKGINFOVersionInfoCode).as_int();
			}
			auto delFilesNode = rootNode.child(cPKGINFODeleteFilesNode);
			if (delFilesNode)
			{
				for (pugi::xml_node n = delFilesNode.child(cPKGINFODeleteFileNode); n; n = n.next_sibling(cPKGINFODeleteFileNode))
				{
					auto attr = n.attribute(cPKGINFODeleteFileName);
					if (attr)
					{
						_filesToDelete.emplace_back(attr.value());
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
						// TODO Bug Package Deinstallation bei Installation (auch neue Dateien werden ueberschrieben)
						// fuer den Moment Package Deinstallation deaktivieren
						//_pkgsToDeinstall.emplace_back(attr.value());
					}
				}
			}
			return true;
		}
	}
	return false;
}


void L3dPackageInfo::SaveDataToDoc(pugi::xml_document& targetDoc) const
{
	pugi::xml_node root = targetDoc.first_child();
	pugi::xml_node versProps = root.append_child(cPKGINFOVersionInfo);
	pugi::xml_attribute attr = versProps.append_attribute(cPKGINFOVersionInfoCode);	
	attr.set_value(_versionCode);

	pugi::xml_node allDel = root.append_child(cPKGINFODeleteFilesNode);
	for_each(begin(_filesToDelete), end(_filesToDelete), [&allDel](const wstring& f)
	{
		auto n = allDel.append_child(cPKGINFODeleteFileNode);
		n.append_attribute(cPKGINFODeleteFileName).set_value(f.c_str());
	});

	pugi::xml_node allDeinst = root.append_child(cPKGINFODeinstallPackages);
	for_each(begin(_pkgsToDeinstall), end(_pkgsToDeinstall), [&allDeinst](const wstring& h)
	{
		auto n = allDeinst.append_child(cPKGINFODeinstallPackageNode);
		n.append_attribute(cPKGINFODeinstallPackageHash).set_value(h.c_str());
	});
}

std::wstring L3dPackageInfo::GetRootNodeName() const
{
	return cPKGINFORoot;
}

}