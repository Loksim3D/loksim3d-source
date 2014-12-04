#ifndef _CRASHRPTSETTINGS_H_
#define _CRASHRPTSETTINGS_H_

#include <LoksimVersionHeader.h>

#define L3D_STRINGIZE(s) L#s
#define L3D_TOSTRING(x) L3D_STRINGIZE(x)

namespace l3d
{
namespace crashreport
{

//const wchar_t CRASH_REPORT_MAIL[] = L"loksim@lukas-haselsteiner.at";	//TODO andere Mail-Adresse?
const wchar_t CRASH_REPORT_URL[] = L"http://loksim3d.lukas-haselsteiner.at/crashrpt/crashrpt.php?version=" L3D_TOSTRING(L3D_VERSION_CODE);
const wchar_t CRASH_REPORT_PRIVACY_POLICY_URL[] = L"http://loksim3d.lukas-haselsteiner.at/crashrpt/privacypolicy.html";    

}
}
#endif