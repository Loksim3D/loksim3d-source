using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Reflection;
using System.ComponentModel;
using System.Globalization;

namespace Loksim3D.WetterEdit
{
    /// <summary>
    /// Kapselt einen Pfad
    /// <br>
    /// Ermöglicht das Zugreifen auf absoluten bzw relativen Pfad
    /// </br>
    /// </summary>
    public class L3dFilePath
    {
        private static L3dFilePath _loksimDir;

        /// <summary>
        /// Loksim3D Pfad
        /// </summary>
        public static L3dFilePath LoksimDirectory 
        {
            get 
            {
                if (L3dFilePath.IsNullOrEmpty(_loksimDir))
                {
                    _loksimDir = new L3dFilePath(Path.GetDirectoryName(Assembly.GetEntryAssembly().Location));
                    try
                    {
                        string[] lines = File.ReadAllLines(_loksimDir.AbsolutePath + Path.DirectorySeparatorChar + "paths.ini");
                        System.Text.RegularExpressions.Regex rgxPath = new System.Text.RegularExpressions.Regex("\\s*MainDir=(.+)");
                        foreach (string l in lines)
                        {
                            System.Text.RegularExpressions.Match m = rgxPath.Match(l);
                            if (m.Success)
                            {
                                string p = m.Groups[1].Value;
                                if (Path.IsPathRooted(p))
                                {
                                    _loksimDir = new L3dFilePath(System.IO.Directory.Exists(p) ? p : Path.GetDirectoryName(p));
                                }
                                else
                                {
                                    _loksimDir = new L3dFilePath(_loksimDir.AbsolutePath + Path.DirectorySeparatorChar + p);
                                }
                            }
                        }
                    }
                    catch (Exception)
                    { 
                    }
                }
                return _loksimDir; 
            }
            set
            {
                _loksimDir = value;
            }
        }

        /// <summary>
        /// Prüft ob <paramref name="l3dPath"/> null ist oder der enthaltene Pfad string.empty ist
        /// </summary>
        /// <param name="l3dPath">Pfad</param>
        /// <returns>true falls null oder leer</returns>
        public static bool IsNullOrEmpty(L3dFilePath l3dPath)
        {
            return l3dPath == null || String.IsNullOrEmpty(l3dPath.AbsolutePath);
        }

        /// <summary>
        /// Erzeugt Pfad relativ zu anderer DAtei
        /// <br>Falls relativer Pfad mit '/' beginnt, wird der Pfad als relativ zum Loksim-Verzeichnis interpretiert</br>
        /// <br>Falls <paramref name="parentFile"/> null ist, wird Pfad relativ zum Loksim-Verzeichnis erzeugt</br>
        /// </summary>
        /// <param name="relativePath">Relativer Pfad</param>
        /// <param name="parentFile">"Eltern-Datei" (kann null sein)</param>
        /// <returns>Erzeugter Pfad</returns>
        public static L3dFilePath CreateRelativeToFile(string relativePath, L3dFilePath parentFile)
        {
            L3dFilePath ret = new L3dFilePath();
            ret.SetPathRelativeToFile(relativePath, parentFile);
            return ret;
        }

        /// <summary>
        /// Erzeugt Pfad relativ zum Loksim-Verzeichnis
        /// </summary>
        /// <param name="relativePath">Pfad relativ zum Loksim-Verzeichnis</param>
        /// <returns>Erzeugter Pfad</returns>
        public static L3dFilePath CreateRelativeToL3dDir(string relativePath)
        {
            L3dFilePath p = new L3dFilePath();
            p.SetPathRelativeToL3dDir(relativePath);
            return p;
        }

        private void SetPathRelativeToL3dDir(string relativePath)
        {
            if (string.IsNullOrEmpty(relativePath))
            {
                _absolutePath = string.Empty;
            }
            else if (Path.IsPathRooted(relativePath) && !relativePath.StartsWith("\\", StringComparison.OrdinalIgnoreCase))
            {
                _absolutePath = relativePath;
            }
            else if (relativePath.StartsWith("\\", StringComparison.OrdinalIgnoreCase))
            {
                _absolutePath = Path.Combine(LoksimDirectory.AbsolutePath, relativePath.Substring(1));
            }
            else
            {
                _absolutePath = Path.Combine(LoksimDirectory.AbsolutePath, relativePath);
            }
        }

        private void SetPathRelativeToFile(string relativePath, L3dFilePath parentFile)
        {
            if (string.IsNullOrEmpty(relativePath))
            {
                _absolutePath = string.Empty;
            }
            else if (L3dFilePath.IsNullOrEmpty(parentFile) || relativePath[0] == '\\')
            {
                SetPathRelativeToL3dDir(relativePath);
            }
            else if (Path.IsPathRooted(relativePath))
            {
                _absolutePath = relativePath;
            }
            else
            {
                _absolutePath = Path.Combine(Path.GetDirectoryName(parentFile.AbsolutePath), relativePath);
            }
            ParentFile = parentFile;
        }

        /// <summary>
        /// "Eltern-Datei"
        /// </summary>
        public L3dFilePath ParentFile { get; set; }

        private string _absolutePath;

        /// <summary>
        /// Absoluter Pfad
        /// </summary>
        public string AbsolutePath 
        {
            get
            {
                return _absolutePath;
            }
        }

        /// <summary>
        /// Liefert Pfad relativ zu angegebener Datei
        /// </summary>
        /// <param name="parentFile">"Eltern-Datei"</param>
        /// <returns>Relativer Pfad zur Datei; Falls diese Datei nicht im gleichen Verzeichnis wie parentFile liegt, wird Pfad relativ zum Loksim-Verzeichnis geliefert</returns>
        public string GetPathRelativeToFile(L3dFilePath parentFile)
        {
            if (!L3dFilePath.IsNullOrEmpty(parentFile))
            {
                string parentDir = parentFile.Directory;
                if (AbsolutePath.StartsWith(parentDir, StringComparison.OrdinalIgnoreCase))
                {
                    return AbsolutePath.Substring(parentDir.Length + 1);
                }
            }
            return PathRelativeToL3dDir;
        }

        /// <summary>
        /// Pfad relativ zu definierter ParentFile
        /// </summary>
        public string PathRelativeToParentFile
        {
            get
            {
                return GetPathRelativeToFile(ParentFile);
            }
        }

        /// <summary>
        /// Pfad relativ zum Loksim-Verzeichnis
        /// </summary>
        public string PathRelativeToL3dDir
        {
            get
            {
                if (AbsolutePath.StartsWith(LoksimDirectory.AbsolutePath, StringComparison.OrdinalIgnoreCase))
                {
                    return AbsolutePath.Substring(LoksimDirectory.AbsolutePath.Length);
                }
                return AbsolutePath;
            }
        }

        /// <summary>
        /// Absoluter Pfad zum Verzeichnis in dem diese Datei liegt
        /// </summary>
        public string Directory
        {
            get
            {
                //if (System.IO.Directory.Exists(AbsolutePath))
                //{
                //    return AbsolutePath;
                //}
                return Path.GetDirectoryName(AbsolutePath);
            }
        }

        public L3dFilePath()
        {
            _absolutePath = string.Empty;
        }

        public L3dFilePath(string absolutePath)
        {
            _absolutePath = Path.GetFullPath(absolutePath);
        }

        public override bool Equals(object obj)
        {
            L3dFilePath o = obj as L3dFilePath;
            return o != null && String.Equals(o.AbsolutePath, this.AbsolutePath);
        }


        public override int GetHashCode()
        {
            return AbsolutePath != null ? AbsolutePath.GetHashCode() : 0;
        }

        public override string ToString()
        {
            return String.Format(CultureInfo.CurrentCulture, "L3dPath [{0}]", AbsolutePath);
        }

        /// <summary>
        /// Dateiname (mit Endung)
        /// </summary>
        public string Filename 
        { 
            get
            {
                return Path.GetFileName(AbsolutePath);
            }
        }

        /// <summary>
        /// Liefert true falls Datei im Filesystem existiert
        /// </summary>
        public bool Exists
        {
            get
            {
                return File.Exists(_absolutePath);
            }
        }
    }
}
