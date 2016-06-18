#pragma once

#include <functional>
#include <string>

#include <boost/filesystem.hpp>

namespace l3d
{
namespace common
{

/**
 * Repräsentiert einen Pfad
 */
class L3dPath
{
public:
	static L3dPath CreateRelativeToFile(const std::wstring& path, const L3dPath& parentFile);
	static L3dPath CreateRelativeToL3dDir(const std::wstring& relativePath);

	static L3dPath CreateRelativeToFile(const std::string& path, const L3dPath& parentFile);
	static L3dPath CreateRelativeToL3dDir(const std::string& relativePath);

	static const L3dPath& GetLoksimDir();
	static void SetLoksimDir(const L3dPath& loksimDir);

private:
	static L3dPath loksimDir_;
	
public:
	L3dPath(const wchar_t *absolutePath);
	L3dPath(std::wstring absolutePath);
	L3dPath(const char* absolutePath);
	L3dPath(std::string absolutePath);
	L3dPath(const boost::filesystem::path& absolutePath);
	L3dPath(boost::filesystem::path&& absolutePath);

	L3dPath(void) = default;

	/**
	 * Liefert absoluten Pfad
	 */
	const boost::filesystem::path& GetAbsolutePath() const;

	/**
	 * Liefert Pfad "Loksim-relativ" zu übergebenen parentPath
	 * Falls Datei in Hierachie weiter oben liegt, wird
	 * GetL3dDirRelativePath() zurückgeliefert
	 */
	std::wstring GetRelativePath(const L3dPath& parentPath) const;

	/**
	 * Liefert Pfad relativ zum Loksim Verzeichnis
	 */
	std::wstring GetL3dDirRelativePath() const;

	bool IsInLoksimDirectory() const;

	/**
	 * Vergleicht zwei Pfade (AboslutePath ==)
	 */
	bool operator==(const L3dPath& other) const;

	/**
	* Vergleicht zwei Pfade auf Ungleichheit (AboslutePath !=)
	*/
	bool operator!=(const L3dPath& other) const;

	/**
	* Vergleicht zwei Pfade (AboslutePath <)
	*/
	bool operator<(const L3dPath& other) const;

	/**
	 * Liefert true falls kein Pfad gespeichert ist
	 */
	bool IsEmpty() const;

	/**
	 * Liefert true falls diese Datei existiert
	 * verwendet boost::filesystem::exists()
	 */
	bool Exists() const;

private:
	void NormalizePath();

	boost::filesystem::path absolutePath_;
};

}
}

namespace std {
	template <>
	class hash<l3d::common::L3dPath>
	{
	public:
		/**
		 * Hasher um L3dPath als Key in Hash-Table verwenden zu können
		 */
		size_t operator()(const l3d::common::L3dPath &path ) const
		{
			hash<wstring> hasher;
			return hasher(path.GetAbsolutePath().wstring());
		}
	};


	template<> 
	struct less<l3d::common::L3dPath>
	{
		bool operator()(const l3d::common::L3dPath& p1, const l3d::common::L3dPath& p2) const
		{
			return p1.GetAbsolutePath() < p2.GetAbsolutePath();
		}
	};
};

