#pragma once

#include <functional>
#include <vector>

#include "pugixml/pugixml.hpp"

#include "L3dPath.h"


namespace l3d
{
namespace common
{
class L3dPath;
}

namespace files
{
namespace helper
{

std::vector<float> ReadFloatVector(const pugi::xml_node& n, const::pugi::char_t* name);
std::vector<int> ReadIntVector(const pugi::xml_node& n, const::pugi::char_t* name);
std::vector<std::wstring> ReadStringVector(const pugi::xml_node& n, const pugi::char_t* name);
std::vector<size_t> ReadUIntVector(const pugi::xml_node& n, const::pugi::char_t* name);
std::wstring ReadString(const pugi::xml_node& n, const::pugi::char_t* name, const std::wstring& defValue = L"");

void WriteUIntVector(const std::vector<size_t>& vec, pugi::xml_node& n, const::pugi::char_t* name);
void WriteStringVector(const std::vector<std::wstring>& vec, pugi::xml_node& n, const pugi::char_t* name);

// Liest einen Enum vom Typ T ein
// Liefert defaultValue falls Enum nicht im angegeben Bereich ist oder Attribut nicht existiert
template <class T>
T ReadIntEnum(const pugi::xml_node& n, const::pugi::char_t* name, const T minValue, const T maxValue, const T defaultValue);

// Speichert ein pugi::xml_document auf sichere Weise:
// Speichert das Dokument zuerst in eine temporäre Datei
// und verschiebt diese Datei dann an den richtigen Ort
// Dies verhindert ''halb-überschriebene'' Dateien bei einem Absturz
bool SaveToFile(const pugi::xml_document& doc, const common::L3dPath& fileName);

// Liest von Datei
// Wirft entsprechende l3d::errors::io_error Execption 
void ReadFromFile(const common::L3dPath& fileName, pugi::xml_document& doc);

// Makes a deep comparison of the two XML Nodes
// Returns true if they are equal
bool CompareXmlNodes(const pugi::xml_node& node1, const pugi::xml_node& node2);


// Template Implementations

// Liest einen Enum vom Typ T ein
// Liefert defaultValue falls Enum nicht im angegeben Bereich ist oder Attribut nicht existiert
template <class T>
T ReadIntEnum(const pugi::xml_node& n, const::pugi::char_t* name, const T minValue, const T maxValue, const T defaultValue)
{
	auto attr = n.attribute(name);
	if (attr) {
		auto intVal = attr.as_int(static_cast<int>(defaultValue));
		if (intVal >= static_cast<int>(minValue) && intVal <= static_cast<int>(maxValue)) {
			return static_cast<T>(intVal);
		}
	}
	return defaultValue;
}
}
}
}