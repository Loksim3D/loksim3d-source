#include "..\Include\lhFile.h"
#include <lhException.h>
#include "..\Include\lhWinAPIG.h"
#include "..\Include\lhWinAPI.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 


//********************************************************************************************************
// �fnet die Datei fileName mit den gew�nschten Attributen (wie CreateFile); die anderen Parameter sind 0)
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
lhWinAPI::File::File(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition)
{
	if (INVALID_HANDLE_VALUE == (m_hFile = CreateFile(fileName.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, 0, NULL)))
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());
}

//********************************************************************************************************
// �fnet die Datei fileName mit den gew�nschten Attributen (wie CreateFile)
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
lhWinAPI::File::File(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		   DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (INVALID_HANDLE_VALUE == (m_hFile = CreateFile(fileName.c_str(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
								 dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile)))
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());
}

//********************************************************************************************************
// Initialisiert die FileKlasse mit dem Handle hFile zu einer Datei (pr�ft auf INVALID_HANDLE_VALUE
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
lhWinAPI::File::File(HANDLE hFile) : m_hFile(hFile)
{
	if (m_hFile == INVALID_HANDLE_VALUE)
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());
}

//******************************
// L�scht die FileKlasse (Handle)
lhWinAPI::File::~File(void)
{
	Close();
}

//*************************************************************
// Liest die gesamte Datei in den ASCII-String dest (byteweise)
// Liefert die Anzahl gelesener Bytes zur�ck
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
DWORD lhWinAPI::File::ReadBytes(std::string& dest) const
{
	BYTE *pBuffer;
	DWORD dwBytesRead;

	ReadUnsafe(&pBuffer, &dwBytesRead);

	dest.assign(reinterpret_cast<char*>(pBuffer), dwBytesRead + 2);
	SAFE_DELETE(pBuffer);
	return dwBytesRead;
}

//***********************************************************
// Schreibt Byte f�r Byte des ASCII-String src in die Datei 
// Liefert die Anzahl geschriebener Bytes zur�ck
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
DWORD lhWinAPI::File::WritesBytes(std::string& src) const
{
	DWORD bytesWritten = 0;

	if (!WriteFile(m_hFile, src.data(), static_cast<DWORD>(src.length()), &bytesWritten, NULL) || bytesWritten != src.length())
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());

	return bytesWritten;
}

//*********************************************************
// Schlie�t den FileHandle(wird von Destruktor auto. gemacht
void lhWinAPI::File::Close()
{
	if (m_hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;

	}
}

//**************************************************************************************************
// Liest die gesamte Datei auf ppBuffer und schreibt die Anzahl gelesener Bytes in dwBytesRead
// (h�ngt automatisch zweil NUL hinten an
// ppBuffer muss mit delete gel�scht werden!!!
void lhWinAPI::File::ReadUnsafe(PBYTE* ppBuffer, PDWORD pdwBytesRead) const
{
	int iFileLength = 0;

	iFileLength = GetFileSize (m_hFile, NULL) ; 
	if (iFileLength == INVALID_FILE_SIZE)
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());

	try
	{
		*ppBuffer = new BYTE[iFileLength + 2];
	}
	catch (std::bad_alloc e)
	{
		throw;
	}

	if (!ReadFile(m_hFile, *ppBuffer, iFileLength, pdwBytesRead, NULL) || iFileLength != *pdwBytesRead)
	{
		SAFE_DELETE(*ppBuffer);
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());
	}

	(*ppBuffer)[iFileLength] = '\0';
	(*ppBuffer)[iFileLength + 1] = '\0';
}