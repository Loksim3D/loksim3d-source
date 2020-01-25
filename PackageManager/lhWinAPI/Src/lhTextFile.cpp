#if defined(UNICODE) || defined(_UNICODE)

#include "..\Include\lhTextFile.h"
#include <lhException.h>
#include "..\Include\lhWinAPIG.h"
#include "..\Include\lhWinAPI.h"
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 

#include <memory>


//********************************************************************************************************
// Öfnet die Datei fileName mit den gewünschten Attributen (wie CreateFile); die anderen Parameter sind 0)
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
lhWinAPI::TextFile::TextFile(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition) : 
			File(fileName, dwDesiredAccess, dwShareMode, dwCreationDisposition)
{
}

//********************************************************************************************************
// Öfnet die Datei fileName mit den gewünschten Attributen (wie CreateFile)
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
lhWinAPI::TextFile::TextFile(std::wstring fileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
						 DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) : 
			File(fileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile)
{
}

//********************************************************************************************************
// Initialisiert die FileKlasse mit dem Handle hFile zu einer Datei (prüft auf INVALID_HANDLE_VALUE
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
lhWinAPI::TextFile::TextFile(HANDLE hFile) : File(hFile)
{
}

//******************************
// Löscht die FileKlasse (Handle)
lhWinAPI::TextFile::~TextFile(void)
{
	Close();
}

//*******************************************
// Bestimmt Codierung des Buffers
lhWinAPI::TextFile::ENCODING lhWinAPI::TextFile::GetEncoding(const PBYTE pBuffer, ENCODING code, PBYTE *ppText, DWORD *pdwFileLength) const
{
	*ppText = pBuffer;
	if (pBuffer[0] == 0xEF && pBuffer[1] == 0xBB && pBuffer[2] == 0xBF)			
	{																			//UTF-8
		*ppText = pBuffer + 3;
		code = UTF8;
	}
	else if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE)																
	{																			//UTF-16
		*ppText = pBuffer + 2;
		code = UTF16;
	}
	else if (pBuffer[0] == 0xFE && pBuffer[1] == 0xFF)								
	{																			//UTF-16BigEndian
		*ppText = pBuffer + 2;
		pdwFileLength -= 2;
		code = UTF16BE;
	}
	else if (pBuffer[0] == 0xFF && pBuffer[1] == 0xFE && pBuffer[2] == 0x00 && pBuffer[3] == 0x00)			
	{																			//UTF-32
		*ppText = pBuffer + 4;
		code = UTF32;
	}
	else if (pBuffer[0] == 0x00 && pBuffer[1] == 0x00 && pBuffer[2] == 0xFE && pBuffer[3] == 0xFF)			
	{																			//UTF-32BigEndian
		*ppText = pBuffer + 4;
		code = UTF32BE;		
	}
	return code;
}

//*****************************************************************************
// Liest die Datei und versucht automatisch den richtigen Zeichensatz zu finden
// pUsedDefaultChar wird true falls ein Zeichen nicht kodiert werden konnte
// Returnt die verwendete Codierung
// wirft lhstd::file_io_error falls etwas schiefgeht
 lhWinAPI::TextFile::ENCODING lhWinAPI::TextFile::ReadText(std::wstring& dest, int Encoding, bool *pUsedDefaultChar) const
{
    DWORD dwFileLength;
    PBYTE pBuffer = 0, pText = 0;
	BOOL bDefault = FALSE;
	ENCODING code = ASCII;

	ReadUnsafe(&pBuffer, &dwFileLength);

	if (dwFileLength < 4)
	{
		pText = pBuffer;
		code = (ENCODING)Encoding;
	}
	else																			//Codierung bestimmen			
	{
		code = GetEncoding(pBuffer, (ENCODING)(Encoding & ~FORCEENCODING), &pText, &dwFileLength);
		if (Encoding & FORCEENCODING)
			code = (ENCODING)(Encoding & ~FORCEENCODING);
	}																																

	switch(code)																	//Codierung anwenden
	{	
	case UTF8:
		lhWinAPI::MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<char*>(pText), -1, dest);
		break;

	case UTF16:
		dest = reinterpret_cast<wchar_t*>(pText);
		break;

	case UTF16BE:
		{																			
			BYTE bySwap;
			DWORD i;
			for (i = 0 ; i < dwFileLength / 2 ; i++)									//Swappen
			{
				bySwap = ((BYTE *)pText)[2 * i];
				((BYTE *)pText)[2 * i] = ((BYTE *)pText)[2 * i + 1];
				((BYTE *)pText)[2 * i + 1] = bySwap;
			 }
			dest = reinterpret_cast<wchar_t*>(pText);
		}
		break;
	
	case UTF32:
		throw lhstd::file_io_error(TEXT("UTF-32 ist derzeit nicht implementiert"));

	case UTF32BE:			
		throw lhstd::file_io_error(TEXT("UTF-32 ist derzeit nicht implementiert"));

	case ASCII:	
	default:
		pText = pBuffer;
		lhWinAPI::MultiByteToWideChar(CP_ACP, 0, reinterpret_cast<char*>(pText), -1, dest);
	}

	SAFE_DELETE(pBuffer);
	if (pUsedDefaultChar)
		*pUsedDefaultChar = (bDefault == TRUE) ? true : false;
	return code;
}

//*****************************************************************
// Schreibt den String text in die Datei mit der Codierung Encoding
// Liefert die Anzahl geschriebener Bytes zurück
// wirft lhstd::file_io_error mit Fehlerbeschreibung von lhWinAPI::GetLastErrorS() falls etwas schiefgeht
 DWORD lhWinAPI::TextFile::WriteText(std::wstring& text, ENCODING Encoding,  bool *pUsedDefaultChar) const
{
	DWORD dwBytesWritten = 0;
	BOOL bDefault = FALSE;
	std::string asciiDest;
	BOOL bWrite = FALSE;

	switch(Encoding)
	{
	case UTF8:
		{
			//BYTE dwByteOrderMark[] = {0xEF, 0xBB, 0xBF };							//ByteOrderMark für UTF8
			//WriteFile(m_hFile, (LPCVOID)dwByteOrderMark, 3, &dwBytesWritten, NULL);
			lhWinAPI::WideCharToMultiByte(CP_UTF8, 0, text.c_str(), asciiDest, 0, 0);
			bWrite = WriteFile(m_hFile, asciiDest.data(), static_cast<DWORD>(asciiDest.length()), &dwBytesWritten, NULL);
		}
		break;

	case UTF16:									//!!!!*****!!!!!!!!
		{
			BYTE dwByteOrderMark[] = {0xFF, 0xFE };									//ByteOrderMark für UTF16
			WriteFile(m_hFile, (LPCVOID)dwByteOrderMark, 2, &dwBytesWritten, NULL);
			bWrite = WriteFile(m_hFile, text.data(), static_cast<DWORD>(text.length() * 2), &dwBytesWritten, NULL);
		}
		break;
 
	case UTF16BE:
		{
			BYTE dwByteOrderMark[] = {0xFE, 0xFF };									//ByteOrderMark für UTF16BE
			WriteFile(m_hFile, (LPCVOID)dwByteOrderMark, 2, &dwBytesWritten, NULL);
			std::unique_ptr<BYTE> pBuffer(new BYTE[text.length() * 2]);
			StringCbCopy(reinterpret_cast<wchar_t*>(pBuffer.get()), text.length() * 2, text.c_str());
			BYTE bySwap;
			for (size_t i = 0 ; i < text.length(); i++)							//Swappen
			{
				bySwap = ((BYTE *)pBuffer.get())[2 * i];
				((BYTE *)pBuffer.get())[2 * i] = ((BYTE *)pBuffer.get())[2 * i + 1];
				((BYTE *)pBuffer.get())[2 * i + 1] = bySwap;
			}
			bWrite = WriteFile(m_hFile, pBuffer.get(), static_cast<DWORD>(text.length() * 2), &dwBytesWritten, NULL);
		}
		break;

	case UTF32:
		throw lhstd::file_io_error(TEXT("UTF-32 ist derzeit nicht implementiert"));
	case UTF32BE:
		throw lhstd::file_io_error(TEXT("UTF-32 ist derzeit nicht implementiert"));

	case ASCII:
	default:
		lhWinAPI::WideCharToMultiByte(CP_ACP, 0, text, asciiDest, NULL, &bDefault);
		bWrite = WriteFile(m_hFile, asciiDest.data(), static_cast<DWORD>(asciiDest.length()), &dwBytesWritten, NULL);
	}
	if (pUsedDefaultChar)
		*pUsedDefaultChar = bDefault ? true : false;

	if (!bWrite)
		throw lhstd::file_io_error(lhWinAPI::GetLastErrorS());

	return dwBytesWritten;
}

#endif