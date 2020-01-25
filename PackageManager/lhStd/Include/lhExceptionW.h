#pragma once

#include <string>

namespace lhstd
{
	//*****************************
	// Die Mutter aller Exceptions
	class exception
	{
		std::wstring m_sMessage;
	public:
		exception() : m_sMessage(L"")
		{
		}
		exception(const std::wstring& message) : m_sMessage(message)
		{
		}
		//**********************
		// Fehlerbeschreibung
		virtual const std::wstring& what() const
		{
			return m_sMessage;
		}

		virtual ~exception() {}
	};

	//****************************************************
	// Logische Fehler (durch Argumentprüfung verhinderbar
	class logic_error : public exception
	{
	public:
		logic_error(const std::wstring& message) : exception(message)
		{
		}
	};

	//****************************************
	// Out of range (index zu groß/klein
	class out_of_range : public logic_error
	{
	public:
		out_of_range(const std::wstring& message) : logic_error(message)
		{
		}
	};

	//*****************************************
	// Falsches Argument / Aufruf nicht möglich
	class invalid_argument : public logic_error
	{
	public:
		invalid_argument(const std::wstring& message) : logic_error(message)
		{
		}
	};

	//***********************************************************
	// Logische Fehler (NICHT durch Argumentprüfung verhinderbar)
	class runtime_error : public exception
	{
	public:
		runtime_error(const std::wstring& message) : exception(message)
		{
		}
	};

	//****************************************************
	// Fehler beim Datei lesen/schreiben/erstellen...
	class file_io_error : public runtime_error
	{
	public:
		file_io_error(const std::wstring& message) : runtime_error(message)
		{
		}
	};

	//****************************************************
	// Fehler beim Internet connecten/lesen/schreiben...
	class internet_error : public runtime_error
	{
	public:
		internet_error(const std::wstring& message) : runtime_error(message)
		{
		}
	};
}
