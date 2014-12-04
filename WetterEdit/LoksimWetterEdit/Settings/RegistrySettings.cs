using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Win32;
using System.Globalization;

namespace Loksim3D.WetterEdit.Settings
{
    /// <summary>
    /// Einstellungen welche in der Registry vorgenommen werden
    /// </summary>
    public class RegistrySettings
    {
        public enum AdminStartMode
        {
            Ask, StartAsAdmin, DoNotStartAsAdmin
        }


        private static RegistrySettings inst = new RegistrySettings();

        private List<L3dFilePath> _recentFiles;

        private RegistrySettings()
        {
        }

        public static RegistrySettings Default
        {
            get { return inst; }
        }

        /// <summary>
        /// Wie soll sich der WetterEditor verhalten wenn er in einem Verzeichnis gestartet wird, wo er keine Schreibrechte besitzt
        /// </summary>
        public AdminStartMode DefaultAdminStartMode
        {
            get
            {
                AdminStartMode ret = RegistrySettings.AdminStartMode.Ask;
                try
                {
                    using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Loksim-Group\WetterEdit\UAC"))
                    {
                        if (key != null)
                        {
                            ret = (AdminStartMode)Enum.Parse(typeof(AdminStartMode), key.GetValue("AdminStartMode", Enum.GetName(typeof(AdminStartMode), ret)).ToString(), true);
                        }
                    }
                }
                catch (Exception)
                {
                    ret = AdminStartMode.Ask;
                }
                return ret;
            }
            set
            {
                using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"Software\Loksim-Group\WetterEdit\UAC"))
                {
                    if (key != null)
                    {
                        key.SetValue("AdminStartMode", Enum.GetName(typeof (AdminStartMode), value));
                    }
                }
            }
        }

        /// <summary>
        /// Standard Dateiautor
        /// </summary>
        public string DefaultFileAuthor
        {
            get
            {
                string ret = string.Empty;
                using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Loksim-Group\LoksimEdit\Editor"))
                {
                    if (key != null)
                    {
                        ret = key.GetValue("DefaultFileAuthor", string.Empty).ToString();
                    }
                }
                return ret;
            }
            set
            {
                using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"Software\Loksim-Group\LoksimEdit\Editor"))
                {
                    if (key != null)
                    {
                        key.SetValue("DefaultFileAuthor", value);
                    }
                }
            }
        }
        
        /// <summary>
        /// Standard Dateiinfo
        /// </summary>
        public string DefaultFileInfo
        {
            get
            {
                string ret = string.Empty;
                using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Loksim-Group\LoksimEdit\Editor"))
                {
                    if (key != null)
                    {
                        ret = key.GetValue("DefaultFileInfo", string.Empty).ToString();
                    }
                }
                return ret;
            }
            set
            {
                using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"Software\Loksim-Group\LoksimEdit\Editor"))
                {
                    if (key != null)
                    {
                        key.SetValue("DefaultFileInfo", value);
                    }
                }
            }
        }
        /// <summary>
        /// Sprache für den WetterEditor
        /// </summary>
        public string UiCulture
        {
            get
            {
                string ret = CultureInfo.CurrentUICulture.Name;
                using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Loksim-Group"))
                {
                    if (key != null)
                    {
                        ret = key.GetValue("UiLanguage", ret).ToString();
                    }
                }
                return ret;
            }
            set
            {
                using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"Software\Loksim-Group"))
                {
                    if (key != null)
                    {
                        key.SetValue("UiLanguage", value);
                    }
                }
            }
        }

        private void LoadRecentFiles()
        {
            _recentFiles = new List<L3dFilePath>();
            using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Loksim-Group\WetterEdit\Recent File List"))
            {
                if (key != null)
                {
                    for (int i = 1; i <= 10; i++)
                    {
                        string s = key.GetValue("File" + i, string.Empty).ToString();
                        if (!string.IsNullOrEmpty(s))
                        {
                            _recentFiles.Add(new L3dFilePath(s));
                        }
                    }
                }
            }        
        }

        /// <summary>
        /// Zuletzte verwendete Dateien
        /// </summary>
        public List<L3dFilePath> RecentFiles
        {
            get
            {
                if (_recentFiles == null)
                {
                    LoadRecentFiles();
                }
                return _recentFiles;
            }
        }

        /// <summary>
        /// Datei zur Liste zuletzt verwendeter Dateien hinzufügen
        /// <br>Falls Datei bereits in Liste vorhanden ist, wird sie vorgereiht</br>
        /// </summary>
        /// <param name="filePath">Dateipfad</param>
        public void AddRecentFile(L3dFilePath filePath)
        {
            if (_recentFiles == null)
            {
                LoadRecentFiles();
            }
            for (int i = 0; i < _recentFiles.Count; i++)
            {
                if (_recentFiles[i].Equals(filePath))
                {
                    _recentFiles.RemoveAt(i);
                    break;
                }
            }
            _recentFiles.Insert(0, filePath);
            if (_recentFiles.Count > 10)
            {
                _recentFiles.RemoveAt(10);
            }
            using (RegistryKey key = Registry.CurrentUser.CreateSubKey(@"Software\Loksim-Group\WetterEdit\Recent File List"))
            {
                if (key != null)
                {
                    for (int i = 0; i < _recentFiles.Count; i++)
                    {
                        key.SetValue("File" + (i + 1), _recentFiles[i].AbsolutePath);
                    }
                }
            }
        }
    }
}
