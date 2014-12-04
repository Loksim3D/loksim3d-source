using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.WindowsAPICodePack.Dialogs;
using Microsoft.Win32;
using System.Windows;
using System.IO;

namespace Loksim3D.WetterEdit.Views
{
    public class TestCl
    {
        public static string Test { get { return "hallO";  } }
    }

    /// <summary>
    /// Helper für FileOpen und FileSave
    /// </summary>
    public static class DialogHelpers
    {
        public static string Test { get { return "hallO"; } }

        /// <summary>
        /// Zeigt Datei-Öffnen Dialog an
        /// </summary>
        /// <param name="selectedFile">Zurzeit gewählte Datei (kann null sein)</param>
        /// <param name="fileExtensions">Dateiendungen</param>
        /// <param name="owner">ParentWindow (kann null sein)</param>
        /// <param name="guid">GUID für FileDialog (Vista+ relevant; OS merkt sich pro GUID wo der Dialog das letzte Mal geöffnet war)</param>
        /// <returns>Gewählte Datei oder null</returns>
        public static string OpenLoksimFile(L3dFilePath selectedFile, IEnumerable<CommonFileDialogFilter> fileExtensions, Window owner, Guid guid = default(Guid))
        {
            return OpenLoksimFile(selectedFile != null ? selectedFile.AbsolutePath : null, fileExtensions, owner, guid);
        }
        /// <summary>
        /// Zeigt Datei-Öffnen Dialog an
        /// </summary>
        /// <param name="selectedFile">Zurzeit gewählte Datei (kann null sein)</param>
        /// <param name="fileExtensions">Dateiendungen</param>
        /// <param name="owner">ParentWindow (kann null sein)</param>
        /// <param name="guid">GUID für FileDialog (Vista+ relevant; OS merkt sich pro GUID wo der Dialog das letzte Mal geöffnet war)</param>
        /// <returns>Gewählte Datei oder null</returns>
        public static string OpenLoksimFile(string selectedFile, IEnumerable<CommonFileDialogFilter> fileExtensions, Window owner, Guid guid = default(Guid))
        {
            if (CommonOpenFileDialog.IsPlatformSupported)
            {
                using (CommonOpenFileDialog dlg = new CommonOpenFileDialog("Loksim3D"))
                {
                    if (!string.IsNullOrEmpty(selectedFile))
                    {
                        dlg.InitialDirectory = Path.GetDirectoryName(selectedFile);
                        dlg.DefaultFileName = Path.GetFileName(selectedFile);
                    }
                     
                    if (Directory.Exists(L3dFilePath.LoksimDirectory.AbsolutePath))
                    {
                        dlg.DefaultDirectory = L3dFilePath.LoksimDirectory.AbsolutePath;
                        dlg.AddPlace(L3dFilePath.LoksimDirectory.AbsolutePath, Microsoft.WindowsAPICodePack.Shell.FileDialogAddPlaceLocation.Bottom);
                        dlg.ShowPlacesList = true;
                    }
                    dlg.CookieIdentifier = guid;
                    if (fileExtensions != null)
                    {
                        foreach (CommonFileDialogFilter f in fileExtensions)
                        {
                            dlg.Filters.Add(f);
                        }
                    }

                    if (owner != null)
                    {
                        if (dlg.ShowDialog(owner) == CommonFileDialogResult.Ok)
                        {
                            return dlg.FileName;
                        }
                    }
                    else
                    {
                        if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
                        {
                            return dlg.FileName;
                        }
                    }
                }
            }
            else
            {
                OpenFileDialog dlg = new OpenFileDialog();
                if (!string.IsNullOrEmpty(selectedFile))
                {
                    dlg.FileName = selectedFile;
                }
                dlg.Title = "Loksim3D";
                if (!string.IsNullOrWhiteSpace(selectedFile))
                {
                    dlg.InitialDirectory = Path.GetDirectoryName(selectedFile);
                }
                if (fileExtensions != null)
                {
                    string f = FileExtensions.CommonDlgFilterToClassicFilter(fileExtensions);
                    dlg.Filter = FileExtensions.CommonDlgFilterToClassicFilter(fileExtensions);
                }
                if (dlg.ShowDialog().GetValueOrDefault(false))
                {
                    return dlg.FileName;
                }
            }
            return null;
        }

        /// <summary>
        /// Zeigt Datei-Speichern Dialog an
        /// </summary>
        /// <param name="selectedFile">Zurzeit gewählte Datei (kann null sein)</param>
        /// <param name="fileExtensions">Dateiendungen</param>
        /// <param name="guid">GUID für FileDialog (Vista+ relevant; OS merkt sich pro GUID wo der Dialog das letzte Mal geöffnet war)</param>
        /// <returns>Gewählte Datei oder null</returns>
        public static L3dFilePath SaveLoksimFile(L3dFilePath selectedFile, IEnumerable<CommonFileDialogFilter> fileExtensions, Guid guid = default(Guid))
        {
            if (CommonSaveFileDialog.IsPlatformSupported)
            {
                using (CommonSaveFileDialog dlg = new CommonSaveFileDialog("Loksim3D"))
                {
                    if (selectedFile != null)
                    {
                        dlg.InitialDirectory = selectedFile.Directory;
                        dlg.DefaultFileName = selectedFile.Filename;
                    }
                    if (Directory.Exists(L3dFilePath.LoksimDirectory.AbsolutePath))
                    {
                        dlg.AddPlace(L3dFilePath.LoksimDirectory.AbsolutePath, Microsoft.WindowsAPICodePack.Shell.FileDialogAddPlaceLocation.Bottom);
                        dlg.ShowPlacesList = true;
                    }
                    dlg.CookieIdentifier = guid;
                    if (fileExtensions != null)
                    {
                        foreach (CommonFileDialogFilter f in fileExtensions)
                        {
                            dlg.Filters.Add(f);
                        }
                        dlg.DefaultExtension = "." + dlg.Filters[0].Extensions[0];
                    }
                    if (dlg.ShowDialog() == CommonFileDialogResult.Ok)
                    {
                        return new L3dFilePath(dlg.FileName);
                    }
                }
            }
            else
            {
                SaveFileDialog dlg = new SaveFileDialog();
                if (selectedFile != null)
                {
                    dlg.FileName = selectedFile.Filename;
                }
                dlg.Title = "Loksim3D";
                if (selectedFile != null)
                {
                    dlg.InitialDirectory = selectedFile.Directory;
                }
                if (fileExtensions != null)
                {
                    dlg.Filter = FileExtensions.CommonDlgFilterToClassicFilter(fileExtensions);
                    dlg.DefaultExt = "." + fileExtensions.First().Extensions[0];
                }
                if (dlg.ShowDialog().GetValueOrDefault(false))
                {
                    return new L3dFilePath(dlg.FileName);
                }
            }
            return null;
        }
    }

    /// <summary>
    /// GUIDs für Dateidialoge
    /// </summary>
    public static class GUIDs
    {
        private static Guid _propdlgFileDokuGuid = new Guid((int) 0x60896fa2, (short)0x48b, (short)0x4786, new byte[] { 0x8b, 0x4, 0x39, 0x7a, 0xba, 0x67, 0x62, 0x3 });

        public static Guid PropDlgFileDokuGuid
        {
            get { return _propdlgFileDokuGuid; }
        }
    }

    /// <summary>
    /// Statische Properties für verschiedenste Dateiendungen (und Kombinationen)
    /// </summary>
    public static class FileExtensions
    {
        private static IEnumerable<CommonFileDialogFilter> _textureFiles =
            new List<CommonFileDialogFilter> { new CommonFileDialogFilter(Resources.Strings.FilesBmp, "*.bmp;*.png;*.tga") };

        private static IEnumerable<CommonFileDialogFilter> _weatherFiles =
            new List<CommonFileDialogFilter> { new CommonFileDialogFilter(Resources.Strings.FilesWeather, "*.l3dwth") };

        private static IEnumerable<CommonFileDialogFilter> _skyFiles =
            new List<CommonFileDialogFilter> { new CommonFileDialogFilter(Resources.Strings.FilesSky, "*.l3dsky") };

        private static IEnumerable<CommonFileDialogFilter> _imageFiles =
            new List<CommonFileDialogFilter> { new CommonFileDialogFilter(Resources.Strings.FilesImages, "*.bmp;*.jpg;*.jpeg;*.gif") };

        private static IEnumerable<CommonFileDialogFilter> _allL3dFiles =
            new List<CommonFileDialogFilter> { new CommonFileDialogFilter(Resources.Strings.FilesLoksim, "*.l3dwth;*.l3dsky") };

        private static IEnumerable<CommonFileDialogFilter> _dokuFiles =
            new List<CommonFileDialogFilter> { new CommonFileDialogFilter(Resources.Strings.FilesDoku, "*.pdf;*.xps;*.txt") };

        public static IEnumerable<CommonFileDialogFilter> AllLoksimFiles
        {
            get { return _allL3dFiles.Union(_skyFiles).Union(_weatherFiles); }
        }

        public static IEnumerable<CommonFileDialogFilter> TextureFiles
        {
            get { return _textureFiles; }
        }

        public static IEnumerable<CommonFileDialogFilter> WeatherFiles
        {
            get { return _weatherFiles; }
        }

        public static IEnumerable<CommonFileDialogFilter> SkyFiles
        {
            get { return _skyFiles; }
        }

        public static IEnumerable<CommonFileDialogFilter> ImageFiles
        {
            get { return _imageFiles; }
        }

        public static IEnumerable<CommonFileDialogFilter> DokuFiles
        {
            get { return _dokuFiles; }
        }

        /// <summary>
        /// Erzeugt String aus CommonFileDialogFilter Collection
        /// </summary>
        /// <param name="l">CommonFileDialogFilter Collection</param>
        /// <returns>Klassischer Filter-String für alte Datei-Dialoge</returns>
        public static string CommonDlgFilterToClassicFilter(IEnumerable<CommonFileDialogFilter> l)
        {
            if (l != null)
            {
                return l.Aggregate("", (str, f) =>
                    str + (str.Length != 0 ? "|" : "") + f.DisplayName + "|" +
                    f.Extensions.Aggregate("", (strExt, ext) =>
                        strExt + (strExt.Length != 0 ? ";" : "") + "*." + ext));
            }
            return string.Empty;
        }
    }
}
