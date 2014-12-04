#pragma once

#include <string>

namespace l3d
{
namespace packageinstaller
{

class RegistryAccess
{
public:
	static std::wstring GetUiCulture();
	static void SetUiCulture(const std::wstring& culture);

private:
	RegistryAccess(void);
	RegistryAccess(const RegistryAccess&);
	const RegistryAccess& operator=(const RegistryAccess&);
	~RegistryAccess(void);
};

}
}