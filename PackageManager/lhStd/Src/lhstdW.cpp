#include "..\Include\lhstdW.h"

//**************************************************************************
// Splittet den String source immer beim Zeichen ch in den Vektor vDest auf
//**************************************************************************
size_t lhstd::split(std::vector<std::wstring>& vDest, const std::wstring& source, wchar_t ch)
{
	std::wstring::size_type pos1 = 0;
	std::wstring::size_type pos2 = source.find(ch);
	size_t count = 0;

	while (pos1 != std::wstring::npos)
	{
		vDest.push_back(source.substr(pos1, pos2 - pos1));
		pos1 = (pos2 == std::wstring::npos) ? std::wstring::npos : pos2 + 1;
		pos2 = source.find(ch, pos2 + 1);
		count++;
	}
	return count;
}
