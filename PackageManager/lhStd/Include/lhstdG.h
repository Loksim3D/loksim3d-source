#pragma once

#include <sstream>

namespace lhstd
{
	//*********************************************************
	//* Wandelt wert vom Typ T in std::basic_string<TCHAR> um  
	template<class T> std::basic_string<TCHAR> String(const T& wert);
	//*******************************************
	// * Liest einen Wert aus einem basic_string 
	template<class T> T Read(const std::basic_string<TCHAR>& string, const T init = T());
	//*****************************************
	//* Liest einen Wert aus einem TCHAR Feld   
	template<class T> T Read(const TCHAR *string, const T init = T());
}

//*****************************************************************
//* Fügt einen String und einen beliebigen anderen Wert aneinander   
template<class T> std::basic_string<TCHAR> operator&(const std::basic_string<TCHAR>& s, const T& v);


//*********************************************************
//* Wandelt wert vom Typ T in std::basic_string<TCHAR> um  
template<class T> std::basic_string<TCHAR> lhstd::String(const T& wert)
{
	std::basic_ostringstream<TCHAR> os;
	std::basic_string<TCHAR> s;

	os << wert;
	s = os.str();
	return s;
}

//*******************************************
// * Liest einen Wert aus einem basic_string 
template<class T> T lhstd::Read(const std::basic_string<TCHAR>& string, const T init)
{
	T wert = init;
	std::basic_istringstream<TCHAR> is;

	is.str(string);
	is >> wert;
	return wert;
}

//*****************************************
//* Liest einen Wert aus einem TCHAR Feld   
template<class T> T lhstd::Read(const TCHAR *string, const T init)
{
	T wert = init;
	std::basic_istringstream<TCHAR> is;

	std::basic_string<TCHAR> s(string);
	is.str(s);
	is >> wert;
	return wert;
}

//*****************************************************************
//* Fügt einen String und einen beliebigen anderen Wert aneinander 
template<class T> std::basic_string<TCHAR> operator&(const std::basic_string<TCHAR>& s, const T& v)
{
	std::basic_stringstream<TCHAR> str;
	str << s << v;
	return str.str();
}
