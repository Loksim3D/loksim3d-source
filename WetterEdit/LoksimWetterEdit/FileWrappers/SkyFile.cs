using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Xml.Linq;
using System.Xml;
using Loksim3D.WetterEdit.LogicWrappers;

namespace Loksim3D.WetterEdit.FileWrappers
{
    /// <summary>
    /// Klasse welche Daten für eine Sky-File .l3dsky speichert
    /// </summary>
    public class SkyFile : UndoAwareFile
    {
        public SkyFile()
        {
            _weatherSets = new UndoAwareObservableCollection<Weather>(this);
        }

        private UndoAwareObservableCollection<Weather> _weatherSets;

        /// <summary>
        /// Enthaltene Wetterdateien
        /// </summary>
        public ObservableCollection<Weather> WeatherSets
        {
            get { return _weatherSets; }
            //set
            //{
            //    if (value != _weatherSets)
            //    {
            //        _weatherSets = value;
            //        NotifyPropertyChanged("WeatherSets");
            //    }
            //}
        }

        protected override void LoadFromDoc(System.Xml.Linq.XDocument doc)
        {
            _weatherSets = new UndoAwareObservableCollection<Weather>(this);
            XElement root = (XElement)doc.FirstNode;
            var sets = root.Element(FileDescriptions.FILE_SKY_EL_WEATHER_SETS);
            if (sets != null)
            {
                foreach (XElement weather in sets.Elements(FileDescriptions.FILE_SKY_EL_WEATHER))
                {
                    _weatherSets.Add(Weather.ReadFromXml(weather, this, OwnPath));
                }
            }
            ClearUndoRedo();
        }

        protected override XDocument SaveToXmlDocument(XElement rootPropsElement, L3dFilePath filePath)
        {
            XElement root = new XElement(FileDescriptions.FILE_SKY_EL_ROOT);
            root.Add(rootPropsElement);

            foreach (var w in WeatherSets.Where(el => el.WeatherFile != null))
            {
                w.WeatherFile.ParentFile = filePath;
            }

            root.Add(new XElement(FileDescriptions.FILE_SKY_EL_WEATHER_SETS, WeatherSets.Select(w => w.ConvertToXml(OwnPath))));

            return new XDocument(root);
        }
    }

    /// <summary>
    /// Representation einer Wetterdatei in Sky-Datei
    /// </summary>
    public class Weather : UndoAwareFilePart
    {
        private L3dFilePath _weatherFile;
        private int _propability;

        public Weather(IUndoManager undoManager) : this(undoManager, null)
        {
        }

        public Weather(IUndoManager undoManager, L3dFilePath weatherFile)
            : base(undoManager)
        {
            _weatherFile = weatherFile;
            _propability = 1;
        }

        /// <summary>
        /// Wetterdatei
        /// </summary>
        public L3dFilePath WeatherFile
        { 
            get
            {
                return _weatherFile;
            }
            set
            {
                if (value != _weatherFile)
                {
                    var oldVal = _weatherFile;
                    _weatherFile = value;
                    NotifyPropertyChanged("WeatherFile", () =>
                        {
                            WeatherFile = oldVal;
                        });
                }
            }
        }

        /// <summary>
        /// Wahrscheinlichkeitsgewichtung der Anwendung dieser Datei bei zufälliger Auswahl
        /// </summary>
        public int Probability 
        {
            get
            {
                return _propability;
            }

            set
            {
                if (value != _propability)
                {
                    var oldVal = _propability;
                    _propability = value;
                    NotifyPropertyChanged("Probability", () =>
                    {
                        Probability = oldVal;
                    });
                }
            }
        }

        public static Weather ReadFromXml(string xmlEl, IUndoManager undoManager, L3dFilePath ownerPath = null)
        {
            try
            {
                return ReadFromXml(XElement.Parse(xmlEl), undoManager, ownerPath);
            }
            catch (XmlException)
            {
            }
            return null;
        }

        public static Weather ReadFromXml(XElement xmlEl, IUndoManager undoManager, L3dFilePath ownerPath = null)
        {
            if (xmlEl != null && xmlEl.Name == FileDescriptions.FILE_SKY_EL_WEATHER)
            {
                XElement props = xmlEl.Element(FileDescriptions.FILE_SKY_EL_WEATHER_PROPS);
                if (props != null)
                {
                    Weather w = new Weather(undoManager);
                    w._propability = props.Attribute(FileDescriptions.FILE_SKY_AT_WEATHER_PROBABILITY).Read<int>(1);
                    w._weatherFile = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_SKY_AT_WEATHER_FILE).Read<string>(), ownerPath);
                    return w;
                }
            }
            return null;
        }

        public XElement ConvertToXml(L3dFilePath ownerPath = null)
        {
            return new XElement(FileDescriptions.FILE_SKY_EL_WEATHER,
                new XElement(FileDescriptions.FILE_SKY_EL_WEATHER_PROPS,
                    new XAttribute(FileDescriptions.FILE_SKY_AT_WEATHER_PROBABILITY, Probability),
                    new XAttribute(FileDescriptions.FILE_SKY_AT_WEATHER_FILE, WeatherFile != null ? WeatherFile.GetPathRelativeToFile(ownerPath) : string.Empty)
                    )
                );
        }
    }
}
