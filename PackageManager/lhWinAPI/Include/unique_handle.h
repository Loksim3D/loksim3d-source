#pragma once

#include <windows.h>

struct handle_traits
{
	static HANDLE invalid() throw()
	{    
		return nullptr;  
	}
	static void close(HANDLE value) throw()
	{    
		CloseHandle(value);  
	}}
;

/**
 * unique_handle verhält sich für handles wie std::unique_ptr<br>
 * Details siehe http://msdn.microsoft.com/en-us/magazine/hh288076.aspx
 */
template <typename Type, typename Traits>
class unique_handle
{
private:
	unique_handle(unique_handle const&);
	unique_handle& operator=(unique_handle const&);

	void close() throw()
	{
		if (*this)
		{
			Traits::close(m_value);
		}
	}
	
	Type m_value;

	struct boolean_struct 
	{ 
		int member; 
	};
	typedef int boolean_struct::*boolean_type;
	
	bool operator==(unique_handle const &);
	bool operator!=(unique_handle const &);

public:
	explicit unique_handle(Type value = Traits::invalid()) throw() : m_value(value)
	{
	}

	unique_handle(unique_handle&& other) throw() :  m_value(other.release())
	{
	}
	
	unique_handle& operator=(unique_handle&& other) throw()
	{
		reset(other.release());
		return *this;
	}

	Type operator->()
	{
		return m_value;
	}

	operator boolean_type() const throw()
	{
		return Traits::invalid() != m_value ? &boolean_struct::member : nullptr;
	}

	Type get() const throw()
	{
		return m_value;
	}

	bool reset(Type value = Traits::invalid()) throw()
	{
		if (m_value != value)
		{
			close();
			m_value = value;
		}
		return *this;
	}

	Type release() throw()
	{
		auto value = m_value;
		m_value = Traits::invalid();
		return value;
	}

	~unique_handle() throw()
	{
		close();
	}
};

typedef unique_handle<HANDLE, handle_traits> handle;