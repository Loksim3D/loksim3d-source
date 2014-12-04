#include "StdAfx.h"
#include "RegistryAccess.h"
#include <Shlwapi.h>
#include <memory>

namespace l3d
{
namespace packageinstaller
{

std::wstring RegistryAccess::GetUiCulture()
{
	DWORD dataSize;
	if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\", L"UiLanguage", nullptr, nullptr, &dataSize) == ERROR_SUCCESS)
	{
		std::unique_ptr<wchar_t> data(new wchar_t[dataSize + 1]);
		if (SHGetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\", L"UiLanguage", nullptr, data.get(), &dataSize) == ERROR_SUCCESS)
		{
			return data.get();
		}
	}
	return L"";
}

void RegistryAccess::SetUiCulture(const std::wstring& culture)
{
	SHSetValue(HKEY_CURRENT_USER, L"Software\\Loksim-Group\\", L"UiLanguage", REG_SZ, culture.data(),
		culture.length() * 2 + 2);
}

RegistryAccess::RegistryAccess(void)
{
}


RegistryAccess::~RegistryAccess(void)
{
}

}
}