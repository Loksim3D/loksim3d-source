#include "stdafx.h"

#include <cwchar>
#include <stdint.h>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>


#include "XmlHelper.h"
#include "FileDescriptions.h"

namespace l3d
{
namespace files
{
namespace helper
{

using namespace std;


// -----------------------------

template<class Type, class WriterFunc>
void WriteVector(const std::vector<Type>& vec, pugi::xml_node& n, const::pugi::char_t* name, WriterFunc func)
{
	std::wstringstream str;
	for (const auto& el : vec) {
		func(str, el);
		str << ';';
	}

	auto s = str.str();
	s = s.substr(0, s.size() - 1);
	n.append_attribute(name).set_value(s.c_str());
}

void WriteUIntVector(const std::vector<size_t>& vec, pugi::xml_node& n, const::pugi::char_t* name)
{
	WriteVector(vec, n, name, [](std::wstringstream& str, const size_t& el) {
		str << el;
	});
}

void WriteStringVector(const std::vector<std::wstring>& vec, pugi::xml_node& n, const pugi::char_t* name)
{
	WriteVector(vec, n, name, [](std::wstringstream& str, const std::wstring& el) {
		str << el;
	});
}


// -------------------------------------

template<class Type, class ReaderFunc>
std::vector<Type> ReadVector(const pugi::xml_node& n, const::pugi::char_t* name, ReaderFunc reader)
{
	std::vector<Type> ret;

	std::wstring val = n.attribute(name).value();
	size_t end = 0;
	while (!val.empty()) {
		end = 0;
		Type t = reader(val, end);
		if (end > 0) {
			ret.emplace_back(t);
			if (val.size() >= end + 1) {
				val = val.substr(end + 1);
			}
			else if (val.size() >= end) {
				val = val.substr(end);
			}
		}
		else {
			val = val.substr(1);
		}
	}

	return ret;
}

std::vector<std::wstring> ReadStringVector(const pugi::xml_node& n, const pugi::char_t* name)
{
	return ReadVector<std::wstring>(n, name, [](const std::wstring& val, size_t& end) {
		while (val[end] != ';' && end < val.size()) {
			end++;
		}
		return boost::trim_copy(val.substr(0, end));
	});
}

std::vector<float> ReadFloatVector(const pugi::xml_node& n, const::pugi::char_t* name)
{
	return ReadVector<float>(n, name, [](const std::wstring& val, size_t& end) {
		try {
			return std::stof(val, &end);
		}
		catch (...) {
			end = 0;
			return 0.0f;
		}
	});
}

std::vector<int> ReadIntVector(const pugi::xml_node& n, const::pugi::char_t* name)
{
	return ReadVector<int>(n, name, [](const std::wstring& val, size_t& end) {
		try {
			return std::stoi(val, &end);
		}
		catch (...) {
			end = 0;
			return 0;
		}
	});
}

std::vector<size_t> ReadUIntVector(const pugi::xml_node& n, const::pugi::char_t* name)
{
	return ReadVector<size_t>(n, name, [](const std::wstring& val, size_t& end) {
		try {
			return std::stoul(val, &end);
		}
		catch (...) {
			end = 0;
			return 0UL;
		}
	});
}


std::wstring ReadString(const pugi::xml_node& n, const::pugi::char_t* name, const std::wstring& defValue /*= L""*/)
{
	auto a = n.attribute(name);
	if (a) {
		std::wstring ret = a.value();
		boost::replace_all(ret, "/r/n", "\r\n");
		boost::replace_all(ret, "/n", "\r\n");
		return ret;
	}
	return defValue;
}

// -------------------------------------



bool SaveToFile(const pugi::xml_document& doc, const common::L3dPath& fileName)
{
	assert(false);
	return false;
}

void ReadFromFile(const common::L3dPath& fileName, pugi::xml_document& doc)
{
	pugi::xml_parse_result result = doc.load_file(fileName.GetAbsolutePath().c_str());

	if (result.status != pugi::status_ok) {
		throw std::exception(fileName.GetAbsolutePath().string().c_str());
	}
}

// -------------------------------------

// Makes a deep comparison of the two XML Nodes
// Returns true if the are equal
bool CompareXmlNodes(const pugi::xml_node& node1, const pugi::xml_node& node2)
{
	if (node1 == node2) {
		return true;
	}
	if (!boost::algorithm::equals(node1.name(), node2.name())) {
		return false;
	}
	auto it1 = node1.attributes_begin();
	auto it2 = node2.attributes_begin();
	while (it1 != node1.attributes_end() && it2 != node2.attributes_end()) {
		if (!boost::algorithm::equals(it1->name(), it2->name()) || !boost::algorithm::equals(it1->value(), it2->value())) {
			return false;
		}
		++it1;
		++it2;
	}
	if (it1 != node1.attributes_end() || it2 != node2.attributes_end()) {
		return false;
	}
	auto nit1 = node1.children().begin();
	auto nit2 = node2.children().begin();
	while (nit1 != node1.children().end() && nit2 != node2.children().end()) {
		if (!CompareXmlNodes(*nit1, *nit2)) {
			return false;
		}
		++nit1;
		++nit2;
	}
	return nit1 == node1.children().end() && nit2 == node2.children().end();
}

}
}
}