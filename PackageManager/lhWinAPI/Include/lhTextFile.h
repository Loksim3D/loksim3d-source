#if defined(UNICODE) || defined(_UNICODE)

#pragma once

#include "lhFile.h"

namespace lhWinAPI
{
//***********************************************************
// Klasse die eine Datei kapselt zum Schreiben/Lesen von Text
class TextFile : public File
{
public:
	//Gibt an welche Encoding benutzt werden soll; Mit | FORCEENCODING bedeutet dass auf jeden Fall diese Codierung benutzt wird
	enum ENCODING { ASCII, UTF8, UTF16, UTF16BE, UTF32, UTF32BE, FORCEENCODING = 32 };

	//********************************************************************************************************
	// Öfnet die Datei fileName mit den gewünschten Attributen (wie CreateFile); die anderen Parameter sind 0)
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	TextFile(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition);
	//*************************************<*******************************************************************
	// Öfnet die Datei fileName mit den gewünschten Attributen (wie CreateFile)
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	TextFile(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
	//********************************************************************************************************
	// Initialisiert die FileKlasse mit dem Handle hFile zu einer Datei (prüft auf INVALID_HANDLE_VALUE
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	TextFile(HANDLE hFile);
	//******************************
	// Löscht die FileKlasse (Handle)
	virtual ~TextFile(void);

	//*****************************************************************************
	// Liest die Datei und versucht automatisch den richtigen Zeichensatz zu finden
	// pUsedDefaultChar wird true falls ein Zeichen nicht kodiert werden konnte
	// Returnt die verwendete Codierung
	// wirft lhstd::file_io_error falls etwas schiefgeht
	ENCODING ReadText(std::wstring& dest, int Encoding = ASCII, bool *pUsedDefaultChar = 0) const;

	//*****************************************************************
	// Schreibt den String text in die Datei mit der Codierung Encoding
	// Liefert die Anzahl geschriebener Bytes zurück
	// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
	DWORD WriteText(std::wstring& text, ENCODING Encoding = ASCII, bool *pUsedDefaultChar=0) const;


private:
	//*******************************************
	// Bestimmt Codierung des Buffers
	ENCODING GetEncoding(const PBYTE pBuffer, ENCODING code, PBYTE *ppText, DWORD *pdwFileLength) const;


private:
	//****************************************************************************************************
	// Copy-Konstruktor u. Zuweisungsoperator (existieren nicht, weil Resource nur 1x genutzt werden kann)
	TextFile(const TextFile& file);
	TextFile& operator=(const TextFile& file);
};
}

#endif