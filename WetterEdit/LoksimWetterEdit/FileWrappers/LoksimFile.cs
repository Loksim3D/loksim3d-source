using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using System.IO;
using System.Globalization;
using Loksim3D.WetterEdit.Win32Wrapper;

namespace Loksim3D.WetterEdit.FileWrappers
{
    /// <summary>
    /// Basisklasse für alle Loksim Dateien
    /// </summary>
    public abstract class LoksimFile : BaseModel
    {
        private string _fileAuthor;
        private string _fileInfo;
        private L3dFilePath _filePicture;
        private L3dFilePath _fileDoku;
        private L3dFilePath _ownPath;

        private string _lastSavedDoc;

        protected LoksimFile()
        {
            _filePicture = new L3dFilePath();
            _fileDoku = new L3dFilePath();
            _fileAuthor = Settings.RegistrySettings.Default.DefaultFileAuthor;
            _fileInfo = Settings.RegistrySettings.Default.DefaultFileInfo;
        }

        public string FileAuthor 
        { 
            get
            {
                return _fileAuthor;
            }
            set
            {
                if (value != _fileAuthor)
                {
                    _fileAuthor = value;
                    NotifyPropertyChanged("FileAuthor");
                }
            }
        }

        public string FileInfo
        {
            get
            {
                return _fileInfo;
            }
            set
            {
                if (value != _fileInfo)
                {
                    _fileInfo = value;
                    NotifyPropertyChanged("FileInfo");
                }
            }
        }

        public L3dFilePath FilePicture
        {
            get
            {
                return _filePicture;
            }
            set
            {
                if (value != _filePicture)
                {
                    _filePicture = value ?? new L3dFilePath();
                    NotifyPropertyChanged("FilePicture");
                }
            }
        }

        public L3dFilePath FileDoku
        {
            get
            {
                return _fileDoku;
            }
            set
            {
                if (value != _fileDoku)
                {
                    _fileDoku = value ?? new L3dFilePath();
                    NotifyPropertyChanged("FileDoku");
                }
            }
        }

        /// <summary>
        /// Pfad von welchem diese Datei geladen wurde oder wohin die Datei zuletzt gespeichert wurde
        /// <br> Kann null sein falls Datei noch nicht gespeichert</br>
        /// </summary>
        public L3dFilePath OwnPath
        {
            get
            {
                return _ownPath;
            }
            private set
            {
                if (value != _ownPath)
                {
                    _ownPath = value;
                    NotifyPropertyChanged("OwnPath");
                }
            }
        }

        /// <summary>
        /// Lädt Daten in diese Instanz von der angegebenen Datei
        /// </summary>
        /// <param name="path">Dateipfad</param>
        public void LoadFromFile(L3dFilePath path)
        {
            if (!L3dFilePath.IsNullOrEmpty(path))
            {
                XDocument doc = XDocument.Load(path.AbsolutePath);
                XElement props = doc.Elements().First().Element(FileDescriptions.FILE_GENERAL_EL_PROPS);
                if (props != null)
                {
                    FileAuthor = props.Attribute(FileDescriptions.FILE_GENERAL_AT_AUTHOR).Read<string>();
                    FileInfo = props.Attribute(FileDescriptions.FILE_GENERAL_AT_INFO).Read<string>();
                    FilePicture = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_GENERAL_AT_PICTURE).Read<string>(), path);
                    FileDoku = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_GENERAL_AT_DOKU).Read<string>(), path);
                }
                OwnPath = path;
                LoadFromDoc(doc);
                _lastSavedDoc = GetOutputString(SaveToXmlDocument(GetRootPropsElement(OwnPath), OwnPath));
            }
        }

        /// <summary>
        /// Prüft ob Datei seit letztem Laden / Speichern verändert wurde
        /// <br>Achtung, relativ aufwändig da Konvertierung in XML erfolgt</br>
        /// </summary>
        /// <returns>true falls Datei verändert wurde</returns>
        public bool WasModifiedSinceLastSave()
        {
            return string.IsNullOrEmpty(_lastSavedDoc) || _lastSavedDoc != GetOutputString(SaveToXmlDocument(GetRootPropsElement(OwnPath), OwnPath));
        }

        /// <summary>
        /// Konkrete Klassen müssen Methode zum Laden implementieren und die Daten von übergebenem XDocument einlesen
        /// </summary>
        /// <param name="doc">XDocument welches eingelesen wurde</param>
        protected abstract void LoadFromDoc(XDocument doc);

        /// <summary>
        /// Konkrete Klassen implementieren diese Methode um die Datei in die XML-Darstellung umzuwandeln
        /// </summary>
        /// <param name="rootPropsElement">XML-Element mit den allgemeinen Eigenschaften (FileAuthor, FileInfo,...)</param>
        /// <param name="filePath">Pfad in welche Datei gespeichert wird</param>
        /// <returns>Erzeugtes XDocument mit allen zu speichernden Daten</returns>
        protected abstract XDocument SaveToXmlDocument(XElement rootPropsElement, L3dFilePath filePath);

        private static string GetOutputString(XDocument xDoc)
        {
            using (StringWriter sw = new StringWriter(CultureInfo.InvariantCulture))
            {
                xDoc.Save(sw);
                return sw.ToString();
            }
        }

        /// <summary>
        /// Speichert die Datei ab
        /// </summary>
        /// <param name="saveToFile">Dateipfad</param>
        public void SaveToFile(L3dFilePath outputPath)
        {
            if (OwnPath != outputPath && !L3dFilePath.IsNullOrEmpty(FilePicture))
            {
                try
                {
                    string newPath = Path.Combine(outputPath.Directory, outputPath.Filename + ".jpg");
                    File.Copy(FilePicture.AbsolutePath, newPath);
                    FilePicture = new L3dFilePath(newPath);
                }
                catch (Exception)
                {
                    FilePicture = new L3dFilePath();
                }
            }

            XDocument xDoc = SaveToXmlDocument(GetRootPropsElement(outputPath), outputPath);
            string output = GetOutputString(xDoc);
            string tmpFile = TempFileUtility.GetTempFileName("tmp", 0, outputPath.Directory);
            if (File.Exists(outputPath.AbsolutePath))
            {
                xDoc.Save(tmpFile);
                File.Replace(tmpFile, outputPath.AbsolutePath, null);
            }
            else
            {
                File.Delete(tmpFile);
                xDoc.Save(outputPath.AbsolutePath);
            }
            _lastSavedDoc = output;
            OwnPath = outputPath;
        }

        /// <summary>
        /// Liefert XElement mit den "allgemeinen" Daten
        /// </summary>
        /// <param name="saveToFile">Pfad in welche Datei gespeichert wird (wichtig für relativen Pfad des FilePictures)</param>
        /// <returns>Erzeugtes XElement</returns>
        protected XElement GetRootPropsElement(L3dFilePath saveToFile)
        {        
            return new XElement(FileDescriptions.FILE_GENERAL_EL_PROPS,
                new XAttribute(FileDescriptions.FILE_GENERAL_AT_AUTHOR, FileAuthor ?? ""),
                new XAttribute(FileDescriptions.FILE_GENERAL_AT_INFO, FileInfo ?? ""),
                new XAttribute(FileDescriptions.FILE_GENERAL_AT_PICTURE, FilePicture.GetPathRelativeToFile(saveToFile)),
                new XAttribute(FileDescriptions.FILE_GENERAL_AT_DOKU, FileDoku.GetPathRelativeToFile(saveToFile)));
        }
    }
}
