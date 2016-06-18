#pragma once

#include <LoksimVersionHeader.h>

namespace l3d
{
	/**
	 * Code der aktuellen Version<br>
	 * zB 2.8 = 280, 2.7.2 = 272
	 */
	const int LOKSIM3D_VERSION_CODE = L3D_VERSION_CODE;

	const wchar_t cFileInfo[] = L"FileInfo";
	const wchar_t  cFileAutor[] = L"FileAuthor";
	const wchar_t  cFilePicture[] = L"FilePicture";
	const wchar_t  cFileProps[] = L"Props";

	// Konstanten in der PackageInfo.xml Datei
	const wchar_t cPKGINFORoot[] = L"PackageInfo";
	const wchar_t cPKGINFODisplayName[] = L"DisplayName";
	const wchar_t cPKGINFOAddonID[] = L"AddonID";
	const wchar_t cPKGINFOAddonVersion[] = L"AddonVersion";
	const wchar_t cPKGINFOVersionInfo[] = L"VersionInfo";
	const wchar_t cPKGINFOVersionInfoCode[] = L"Code";
	const wchar_t cPKGINFODeleteFilesNode[] = L"DeleteFiles";
	const wchar_t cPKGINFODeleteFileNode[] = L"DeleteFile";
	const wchar_t cPKGINFODeleteFileName[] = L"Name";
	const wchar_t cPKGINFODeinstallPackages[] = L"DeinstallPackages";
	const wchar_t cPKGINFODeinstallPackageNode[] = L"DeinstallPackage";
	const wchar_t cPKGINFODeinstallPackageHash[] = L"Hash";
	const wchar_t cPKGINFODeinstallAddonID[] = L"AddonID";
}