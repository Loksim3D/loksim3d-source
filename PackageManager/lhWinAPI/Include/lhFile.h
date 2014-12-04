#if defined(UNICODE) || defined(_UNICODE)

#pragma once

#include <windows.h>
#include <string>

#ifndef SAFE_DELETE
	#define SAFE_DELETE(x) if (x != 0) delete x; x = 0;
#endif

namespace lhWinAPI
{
//*******************************
// Klasse die eine Datei kapselt
class File
{
public:
	//********************************************************************************************************
	// �fnet die Datei fileName mit den gew�nschten Attributen (wie CreateFile); die anderen Parameter sind 0)
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	File(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition);
	//********************************************************************************************************
	// �fnet die Datei fileName mit den gew�nschten Attributen (wie CreateFile)
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	File(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	//********************************************************************************************************
	// Initialisiert die FileKlasse mit dem Handle hFile zu einer Datei (pr�ft auf INVALID_HANDLE_VALUE
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	File(HANDLE hFile);

	//******************************
	// L�scht die FileKlasse (Handle)
	virtual ~File(void);
	//*********************************************************
	// Schlie�t den FileHandle(wird von Destruktor auto. gemacht
	void Close();

	//*************************************************************
	// Liest die gesamte Datei in den ASCII-String dest (byteweise)
	// Liefert die Anzahl gelesener Bytes zur�ck
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	DWORD ReadBytes(std::string& dest) const;

	//***********************************************************
	// Schreibt Byte f�r Byte des ASCII-String src in die Datei 
	// Liefert die Anzahl geschriebener Bytes zur�ck
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	DWORD WritesBytes(std::string& src) const;

	//**************************
	// Liefert Handle auf File
	inline HANDLE GetHandle() const { return m_hFile; }

protected:
	//**************************************************************************************************
	// Liest die gesamte Datei auf ppBuffer und schreibt die Anzahl gelesener Bytes in dwBytesRead
	// (h�ngt automatisch zweil NUL hinten an
	// ppBuffer muss mit delete gel�scht werden!!!
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	void ReadUnsafe(PBYTE* ppBuffer, PDWORD pdwBytesRead) const;

	HANDLE m_hFile;

private:
	//****************************************************************************************************
	// Copy-Konstruktor u. Zuweisungsoperator (existieren nicht, weil Resource nur 1x genutzt werden kann)
	File(const File& file);
	File& operator=(const File& file);
};
}

#endif