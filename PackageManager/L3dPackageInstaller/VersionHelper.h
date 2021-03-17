#pragma once

namespace l3d::packageinstaller {
/**
 * @brief returns the version code calculated by checking the file version of Loksim3D.exe or LoksimEdit.exe in the same directory like this exe.
 * If the version code cannot be determined using the .exe files, the L3D_VERSION_CODE constant is returned
 * @return version code determined by checking Loksim3D.exe or LoksimEdit.exe
*/
int GetLoksimVersionCodeFromLoksimExe();
}