using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml.Linq;
using System.ComponentModel;
using System.Collections.ObjectModel;
using System.Xml;
using Loksim3D.WetterEdit.LogicWrappers;

namespace Loksim3D.WetterEdit.FileWrappers
{
    /// <summary>
    /// Representation einer Wetterdatei .l3dwth
    /// </summary>
    public class WeatherFile : UndoAwareFile
    {
        public WeatherFile()
        {
            _illumination = new AmbientIllumination(this);
            _weatherSet = new UndoAwareObservableCollection<WeatherTimeRange>(this);
        }

        /// <summary>
        /// Erstellt eine neue Wetterdatei mit zwei Standard-Zeitbereichen
        /// </summary>
        /// <returns>Standard Wetterdatei</returns>
        public static WeatherFile CreateNewDefaultFile()
        {
            WeatherFile f = new WeatherFile();

            WeatherTimeRange r1 = new WeatherTimeRange(f);
            r1.Begin = new TimeSpan(17, 0, 0);
            r1.End = new TimeSpan(9, 0, 0);
            WeatherTimeRange r2 = new WeatherTimeRange(f);
            r2.Begin = new TimeSpan(3, 0, 0);
            r2.End = new TimeSpan(23, 0, 0);
            using (f.EnterNoUndoSection())
            {
                f.WeatherSet.Add(r1);
                f.WeatherSet.Add(r2);
            }
            return f;
        }

        private string _header;

        /// <summary>
        /// Kurze Beschreibung der Wetterdatei zur Anzeige bei der Auswahl im Loksim3D
        /// </summary>
        public string Header
        {
            get { return _header; }
            set
            {
                if (value != _header)
                {
                    var oldVal = _header;
                    _header = value;
                    NotifyPropertyChanged("Header", () =>  Header = oldVal);
                }
            }
        }
        
        private AmbientIllumination _illumination;

        /// <summary>
        /// Beleuchtungseigenschaften
        /// </summary>
        public AmbientIllumination Illumination
        {
            get { return _illumination; }
            set
            {
                if (value != _illumination)
                {
                    var oldVal = _illumination;
                    _illumination = value;
                    NotifyPropertyChanged("Illumination", () => Illumination = oldVal);
                }
            }
        }


        private UndoAwareObservableCollection<WeatherTimeRange> _weatherSet;

        /// <summary>
        /// Zeitbereiche
        /// </summary>
        public ObservableCollection<WeatherTimeRange> WeatherSet
        {
            get { return _weatherSet; }
            //set
            //{
            //    if (value != _weatherSet)
            //    {
            //        _weatherSet = value;
            //        NotifyPropertyChanged("WeatherSet");
            //    }
            //}
        }

        protected override void LoadFromDoc(System.Xml.Linq.XDocument doc)
        {
            _weatherSet = new UndoAwareObservableCollection<WeatherTimeRange>(this);
            //WeatherSet.AllowNew = false;
            XElement root = (XElement)doc.FirstNode;

            XElement rootProps = doc.Elements().First().Element(FileDescriptions.FILE_GENERAL_EL_PROPS);
            if (rootProps != null)
            {
                _header = rootProps.Attribute(FileDescriptions.FILE_WEATHER_AT_PROPS_HEADER).Read<string>(string.Empty);
            }

            XElement illumination = root.Element(FileDescriptions.FILE_WEATHER_EL_ILLUMINATION);
            Illumination = AmbientIllumination.ReadFromXml(illumination, this, OwnPath);
            if (Illumination == null)
            {
                Illumination = new AmbientIllumination(this);
            }

            var set = root.Element(FileDescriptions.FILE_WEATHER_EL_WEATHER_SET);
            if (set != null)
            {
                List<WeatherTimeRange> tempL = new List<WeatherTimeRange>();
                foreach (XElement timeRange in set.Elements(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE))
                {
                    WeatherTimeRange r = WeatherTimeRange.ReadFromXml(timeRange, this, OwnPath);
                    if (r != null)
                    {
                        tempL.Add(r);
                    }
                }
                foreach (var it in tempL.OrderBy(w => w.Begin))
                {
                    _weatherSet.Add(it);
                }
            }

            ClearUndoRedo();
        }

        protected override XDocument SaveToXmlDocument(XElement rootPropsElement, L3dFilePath filePath)
        {
            XElement root = new XElement(FileDescriptions.FILE_WEATHER_EL_ROOT);
            if (!string.IsNullOrWhiteSpace(_header))
            {
                rootPropsElement.SetAttributeValue(FileDescriptions.FILE_WEATHER_AT_PROPS_HEADER, _header);
                root.Add(rootPropsElement);
            }

            root.Add(Illumination.ConvertToXml());

            foreach (var w in WeatherSet)
            {
                foreach (var t in w.AllTextures.Where(el => el != null))
                {
                    t._texture.ParentFile = filePath;
                }
            }

            root.Add(new XElement(FileDescriptions.FILE_WEATHER_EL_WEATHER_SET, WeatherSet.Select(w => 
                        w.ConvertToXml(filePath)
                    )));

            return new XDocument(root);
        }
    }

    /// <summary>
    /// Eigenschaften Beleuchtung
    /// </summary>
    public class AmbientIllumination : UndoAwareFilePart
    {    
        public AmbientIllumination(IUndoManager undoManager) : base(undoManager)
        {
            _dawnBegin = new TimeSpan(3, 0, 0);
            _dawnEnd = new TimeSpan(9, 0, 0);
            _duskBegin = new TimeSpan(17, 0, 0);
            _duskEnd = new TimeSpan(23, 0, 0);
            _brightnessDay = 1;
            _brightnessNight = 0;
        }


        private TimeSpan _dawnBegin;

        /// <summary>
        /// Beginn Morgengrauen
        /// </summary>
        public TimeSpan DawnBegin
        {
            get { return _dawnBegin; }
            set
            {
                if (value != _dawnBegin)
                {
                    var oldVal = _dawnBegin;
                    _dawnBegin = value;
                    NotifyPropertyChanged("DawnBegin", () => DawnBegin = oldVal);                    
                }
            }
        }

        private TimeSpan _dawnEnd;

        /// <summary>
        /// Ende Morgengrauen
        /// </summary>
        public TimeSpan DawnEnd
        {
            get { return _dawnEnd; }
            set
            {
                if (value != _dawnEnd)
                {
                    var oldVal = _dawnEnd;
                    _dawnEnd = value;
                    NotifyPropertyChanged("DawnEnd", () => DawnEnd = oldVal);
                }
            }
        }

        private TimeSpan _duskBegin;

        /// <summary>
        /// Beginn Abenddämmerung
        /// </summary>
        public TimeSpan DuskBegin
        {
            get { return _duskBegin; }
            set
            {
                if (value != _duskBegin)
                {
                    var oldVal = _duskBegin;
                    _duskBegin = value;
                    NotifyPropertyChanged("DuskBegin", () => DuskBegin = oldVal);
                }
            }
        }

        private TimeSpan _duskEnd;

        /// <summary>
        /// Ende Abenddämmerung
        /// </summary>
        public TimeSpan DuskEnd
        {
            get { return _duskEnd; }
            set
            {
                if (value != _duskEnd)
                {
                    var oldVal = _duskEnd;
                    _duskEnd = value;
                    NotifyPropertyChanged("DuskEnd", () => DuskEnd = oldVal);
                }
            }
        }

        private float _brightnessDay;

        /// <summary>
        /// Helligkeit Tag
        /// </summary>
        public float BrightnessDay
        {
            get { return _brightnessDay; }
            set
            {
                if (value != _brightnessDay)
                {
                    var oldVal = _brightnessDay;
                    _brightnessDay = value;
                    NotifyPropertyChanged("BrightnessDay", () => BrightnessDay = oldVal);
                }
            }
        }
        
        private float _brightnessNight;

        /// <summary>
        /// Helligkeit Nacht
        /// </summary>
        public float BrightnessNight
        {
            get { return _brightnessNight; }
            set
            {
                if (value != _brightnessNight)
                {
                    var oldVal = _brightnessNight;
                    _brightnessNight = value;
                    NotifyPropertyChanged("BrightnessNight", () => BrightnessNight = oldVal);
                }
            }
        }

        public static AmbientIllumination ReadFromXml(XElement xmlEl, IUndoManager undoManager, L3dFilePath ownerPath = null)
        {
            if (xmlEl != null && xmlEl.Name == FileDescriptions.FILE_WEATHER_EL_ILLUMINATION)
            {
                XElement illuminationProps = xmlEl.Element(FileDescriptions.FILE_WEATHER_EL_ILLUMINATION_PROPS);
                if (illuminationProps != null)
                {
                    AmbientIllumination ill = new AmbientIllumination(undoManager);
                    ill._brightnessDay = illuminationProps.Attribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_BRIGHTNESS_DAY).Read<float>(1.0f);
                    ill._brightnessNight = illuminationProps.Attribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_BRIGHTNESS_NIGHT).Read<float>(0.0f);
                    ill._dawnBegin = illuminationProps.Attribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DAWN_BEGIN).ReadTime(new TimeSpan(3, 0, 0));
                    ill._dawnEnd = illuminationProps.Attribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DAWN_END).ReadTime(new TimeSpan(6, 0, 0));
                    ill._duskBegin = illuminationProps.Attribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DUSK_BEGIN).ReadTime(new TimeSpan(17, 0, 0));
                    ill._duskEnd = illuminationProps.Attribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DUSK_END).ReadTime(new TimeSpan(23, 0, 0));
                    return ill;
                }
            }
            return null;
        }

        public XElement ConvertToXml()
        {
            return new XElement(FileDescriptions.FILE_WEATHER_EL_ILLUMINATION,
                new XElement(FileDescriptions.FILE_WEATHER_EL_ILLUMINATION_PROPS,
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_BRIGHTNESS_DAY, BrightnessDay),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_BRIGHTNESS_NIGHT, BrightnessNight),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DAWN_BEGIN, DawnBegin.TotalSeconds),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DAWN_END, DawnEnd.TotalSeconds),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DUSK_BEGIN, DuskBegin.TotalSeconds),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_ILLUMINATION_DUSK_END, DuskEnd.TotalSeconds)));
        }
    }

    /// <summary>
    /// Richtungen innerhalb Skybox
    /// </summary>
    public enum SkyBoxDirection
    {   
        North=0, East=1, South=2, West=3, Above=4
    }

    /// <summary>
    /// Zeitbereich innerhalb Wetterdatei
    /// </summary>
    public class WeatherTimeRange : UndoAwareFilePart
    {
        internal class TextureEntry
        {
            public L3dFilePath _texture;
            public float _tileU;
            public float _tileV;
        }


        private List<TextureEntry> _textures;        

        public WeatherTimeRange(IUndoManager undoManager, L3dFilePath parentFile = null) : base(undoManager)
        {
            _sightDistance = 10000;
            _brightnessFactor = 1.0f;
            _variables = new UndoAwareObservableCollection<L3dVariable>(undoManager);
            _textures = new List<TextureEntry>(5);
            for (int i = 0; i < 5; i++)
            {
                TextureEntry en = new TextureEntry();
                en._texture = L3dFilePath.CreateRelativeToFile(string.Empty, parentFile);
                en._tileU = 1;
                en._tileV = 1;
                _textures.Add(en);
            }
        }

        /// <summary>
        /// Liste aller Texturen für Skybox (Liste enthält immer 5 Elemente)
        /// </summary>
        internal IEnumerable<TextureEntry> AllTextures
        {
            get { return _textures; }
        }
        
        /// <summary>
        /// Skybox Textur für bestimmte Richtung
        /// </summary>
        /// <param name="dir">Richtung für welche Skybox-Textur geliefert werden soll</param>
        /// <returns>Skybox-Textur für Richtung</returns>
        public L3dFilePath GetSkyboxTexture(SkyBoxDirection dir)
        {
            return _textures[(int)dir]._texture;
        }

        /// <summary>
        /// Skybox Tile U für bestimmte Richtung
        /// </summary>
        /// <param name="dir">Richtung für welche Tile-Faktor der Skybox-Textur geliefert werden soll</param></param>
        /// <returns>Skybox-Tile-Faktor für Richtung</returns>
        public float GetSkyboxTileU(SkyBoxDirection dir)
        {
            return _textures[(int)dir]._tileU;
        }

        /// <summary>
        /// Skybox Tile V für bestimmte Richtung
        /// </summary>
        /// <param name="dir">Richtung für welche Tile-Faktor der Skybox-Textur geliefert werden soll</param></param>
        /// <returns>Skybox-Tile-Faktor für Richtung</returns>
        public float GetSkyboxTileV(SkyBoxDirection dir)
        {
            return _textures[(int)dir]._tileV;
        }


        private TimeSpan _begin;

        /// <summary>
        /// Zeitpunkt ab welchem Zeitbereich gültig ist
        /// </summary>
        public TimeSpan Begin
        {
            get { return _begin; }
            set
            {
                if (value != _begin)
                {
                    var oldVal = _begin;
                    _begin = value;
                    NotifyPropertyChanged("Begin", () => Begin = oldVal);
                }
            }
        }

        private TimeSpan _end;

        /// <summary>
        /// Zeitpunkt bis dem Zeitbereich gültig ist
        /// </summary>
        public TimeSpan End
        {
            get { return _end; }
            set
            {
                if (value != _end)
                {
                    var oldVal = _end;
                    _end = value;
                    NotifyPropertyChanged("End", () => End = oldVal);
                }
            }
        }

        /// <summary>
        /// Dauer des Zeitbereihs
        /// </summary>
        public TimeSpan Length
        {
            get 
            {
                return new TimeSpan(0, 0, (int)Math.Abs((End - Begin).TotalSeconds));
           }
        }


        private int _sightDistance;

        /// <summary>
        /// Sichtweite in Meter
        /// </summary>
        public int SightDistance
        {
            get { return _sightDistance; }
            set
            {
                if (value != _sightDistance)
                {
                    var oldVal = _sightDistance;
                    _sightDistance = value;
                    NotifyPropertyChanged("SightDistance", () => SightDistance = oldVal);
                }
            }
        }

        private float _brightnessFactor;

        /// <summary>
        /// Faktor mit welchem Helligkeit multipliziert wird
        /// </summary>
        public float BrightnessFactor
        {
            get { return _brightnessFactor; }
            set
            {
                if (value != _brightnessFactor)
                {
                    var oldVal = _brightnessFactor;
                    _brightnessFactor = value;
                    NotifyPropertyChanged("BrightnessFactor", () => BrightnessFactor = oldVal);
                }
            }
        }

        /// <summary>
        /// Sky-Box Textur Sichtrichtung Norden
        /// </summary>
        public L3dFilePath TextureNorth
        {
            get { return _textures[0]._texture; }
            set
            {
                if (value != _textures[0]._texture)
                {
                    var oldVal = _textures[0]._texture;                    
                    _textures[0]._texture = value;
                    NotifyPropertyChanged("TextureNorth", () => TextureNorth = oldVal);

                    if (!L3dFilePath.IsNullOrEmpty(value))                 
                    {
                        string ext = System.IO.Path.GetExtension(value.Filename);
                        string name = System.IO.Path.GetFileNameWithoutExtension(value.Filename);
                        if (name.EndsWith("_north", StringComparison.Ordinal))
                        {
                            string firstPart = value.AbsolutePath.Substring(0, value.AbsolutePath.Length - ("_north" + ext).Length);
                            if (L3dFilePath.IsNullOrEmpty(TextureEast) || (value.Exists && !TextureEast.Exists))
                            {
                                TextureEast = L3dFilePath.CreateRelativeToFile(firstPart + "_east" + ext, value.ParentFile);
                            }
                            if (L3dFilePath.IsNullOrEmpty(TextureSouth) || (value.Exists && !TextureEast.Exists))
                            {
                                TextureSouth = L3dFilePath.CreateRelativeToFile(firstPart + "_south" + ext, value.ParentFile);
                            }
                            if (L3dFilePath.IsNullOrEmpty(TextureWest) || (value.Exists && !TextureEast.Exists))
                            {
                                TextureWest = L3dFilePath.CreateRelativeToFile(firstPart + "_west" + ext, value.ParentFile);
                            }
                            if (L3dFilePath.IsNullOrEmpty(TextureAbove) || (value.Exists && !TextureEast.Exists))
                            {
                                TextureAbove = L3dFilePath.CreateRelativeToFile(firstPart + "_above" + ext, value.ParentFile);
                            }
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Nord-Textur der Skybox wird sooft in X-Richtung wiederholt
        /// </summary>
        public float TextureNorthTileU
        {
            get { return _textures[0]._tileU; }
            set
            {
                if (value != _textures[0]._tileU)
                {
                    var oldVal = _textures[0]._tileU;
                    _textures[0]._tileU = value;
                    NotifyPropertyChanged("TextureNorthTileU", () => TextureNorthTileU = oldVal);
                }
            }
        }

        /// <summary>
        /// Nord-Textur der Skybox wird sooft in Y-Richtung wiederholt
        /// </summary>
        public float TextureNorthTileV
        {
            get { return _textures[0]._tileV; }
            set
            {
                if (value != _textures[0]._tileV)
                {
                    var oldVal = _textures[0]._tileV;
                    _textures[0]._tileV = value;
                    NotifyPropertyChanged("TextureNorthTileV", () => TextureNorthTileV = oldVal);
                }
            }
        }

        /// <summary>
        /// Sky-Box Textur Sichtrichtung Osten
        /// </summary>
        public L3dFilePath TextureEast
        {
            get { return _textures[1]._texture; }
            set
            {
                if (value != _textures[1]._texture)
                {
                    var oldVal = _textures[1]._texture;
                    _textures[1]._texture = value;
                    NotifyPropertyChanged("TextureEast", () => TextureEast = oldVal);
                }
            }
        }

        /// <summary>
        /// Ost-Textur der Skybox wird sooft in X-Richtung wiederholt
        /// </summary>
        public float TextureEastTileU
        {
            get { return _textures[1]._tileU; }
            set
            {
                if (value != _textures[1]._tileU)
                {
                    var oldVal = _textures[1]._tileU;
                    _textures[1]._tileU = value;
                    NotifyPropertyChanged("TextureEastTileU", () => TextureEastTileU = oldVal);
                }
            }
        }

        /// <summary>
        /// Ost-Textur der Skybox wird sooft in Y-Richtung wiederholt
        /// </summary>
        public float TextureEastTileV
        {
            get { return _textures[1]._tileV; }
            set
            {
                if (value != _textures[1]._tileV)
                {
                    var oldVal = _textures[1]._tileV;
                    _textures[1]._tileV = value;
                    NotifyPropertyChanged("TextureEastTileV", () => TextureEastTileV = oldVal);
                }
            }
        }

        /// <summary>
        /// Sky-Box Textur Sichtrichtung Süden
        /// </summary>
        public L3dFilePath TextureSouth
        {
            get { return _textures[2]._texture; }
            set
            {
                if (value != _textures[2]._texture)
                {
                    var oldVal = _textures[2]._texture;
                    _textures[2]._texture = value;
                    NotifyPropertyChanged("TextureSouth", () => TextureSouth = oldVal);
                }
            }
        }

        /// <summary>
        /// Süd-Textur der Skybox wird sooft in X-Richtung wiederholt
        /// </summary>
        public float TextureSouthTileU
        {
            get { return _textures[2]._tileU; }
            set
            {
                if (value != _textures[2]._tileU)
                {
                    var oldVal = _textures[2]._tileU;
                    _textures[2]._tileU = value;
                    NotifyPropertyChanged("TextureSouthTileU", () => TextureSouthTileU = oldVal);
                }
            }
        }

        /// <summary>
        /// Süd-Textur der Skybox wird sooft in Y-Richtung wiederholt
        /// </summary>
        public float TextureSouthTileV
        {
            get { return _textures[2]._tileV; }
            set
            {
                if (value != _textures[2]._tileV)
                {
                    var oldVal = _textures[2]._tileV;
                    _textures[2]._tileV = value;
                    NotifyPropertyChanged("TextureSouthTileV", () => TextureSouthTileV = oldVal);
                }
            }
        }

        /// <summary>
        /// Sky-Box Textur Sichtrichtung Westen
        /// </summary>
        public L3dFilePath TextureWest
        {
            get { return _textures[3]._texture; }
            set
            {
                if (value != _textures[3]._texture)
                {
                    var oldVal = _textures[3]._texture;
                    _textures[3]._texture = value;
                    NotifyPropertyChanged("TextureWest", () => TextureWest = oldVal);
                }
            }
        }

        /// <summary>
        /// West-Textur der Skybox wird sooft in X-Richtung wiederholt
        /// </summary>
        public float TextureWestTileU
        {
            get { return _textures[3]._tileU; }
            set
            {
                if (value != _textures[3]._tileU)
                {
                    var oldVal = _textures[3]._tileU;
                    _textures[3]._tileU = value;
                    NotifyPropertyChanged("TextureWestTileU", () => TextureWestTileU = oldVal);
                }
            }
        }

        /// <summary>
        /// West-Textur der Skybox wird sooft in Y-Richtung wiederholt
        /// </summary>
        public float TextureWestTileV
        {
            get { return _textures[3]._tileV; }
            set
            {
                if (value != _textures[3]._tileV)
                {
                    var oldVal = _textures[3]._tileV;
                    _textures[3]._tileV = value;
                    NotifyPropertyChanged("TextureWestTileV", () => TextureWestTileV = oldVal);
                }
            }
        }

        /// <summary>
        /// Sky-Box Textur Sichtrichtung Oben
        /// </summary>
        public L3dFilePath TextureAbove
        {
            get { return _textures[4]._texture; }
            set
            {
                if (value != _textures[4]._texture)
                {
                    var oldVal = _textures[4]._texture;
                    _textures[4]._texture = value;
                    NotifyPropertyChanged("TextureAbove", () => TextureAbove = oldVal);
                }
            }
        }

        /// <summary>
        /// Top-Textur der Skybox wird sooft in X-Richtung wiederholt
        /// </summary>
        public float TextureAboveTileU
        {
            get { return _textures[4]._tileU; }
            set
            {
                if (value != _textures[4]._tileU)
                {
                    var oldVal = _textures[4]._tileU;
                    _textures[4]._tileU = value;
                    NotifyPropertyChanged("TextureAboveTileU", () => TextureAboveTileU = oldVal);
                }
            }
        }

        /// <summary>
        /// Top-Textur der Skybox wird sooft in Y-Richtung wiederholt
        /// </summary>
        public float TextureAboveTileV
        {
            get { return _textures[4]._tileV; }
            set
            {
                if (value != _textures[4]._tileV)
                {
                    var oldVal = _textures[4]._tileV;
                    _textures[4]._tileV = value;
                    NotifyPropertyChanged("TextureAboveTileV", () => TextureAboveTileV = oldVal);
                }
            }
        }
        
        private UndoAwareObservableCollection<L3dVariable> _variables;

        /// <summary>
        /// Definierte Variablen in diesem Zeitbereich
        /// </summary>
        public ObservableCollection<L3dVariable> Variables
        {
            get { return _variables; }
            //set
            //{
            //    if (value != _variables)
            //    {
            //        _variables = value;
            //        NotifyPropertyChanged("Variables");
            //    }
            //}
        }

        private bool _snowfall;

        /// <summary>
        /// Schneefall in diesem Zeitbereich
        /// </summary>
        public bool Snowfall
        {
            get { return _snowfall; }
            set
            {
                if (_snowfall != value)
                {
                    var oldVal = _snowfall;
                    _snowfall = value;
                    NotifyPropertyChanged(() => Snowfall, () => Snowfall = oldVal);
                }
            }
        }

        public static WeatherTimeRange ReadFromXml(string xmlEl, IUndoManager undoManager, L3dFilePath ownerPath = null)
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


        public static WeatherTimeRange ReadFromXml(XElement xmlEl, IUndoManager undoManager, L3dFilePath ownerPath = null)
        {
            if (xmlEl != null && xmlEl.Name == FileDescriptions.FILE_WEATHER_EL_TIME_RANGE)
            {
                XElement props = xmlEl.Element(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_PROPS);
                if (props != null)
                {
                    WeatherTimeRange t = new WeatherTimeRange(undoManager, ownerPath);
                    using (var tk = undoManager.EnterNoUndoSection())
                    {
                        t._begin = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_BEGIN).ReadTime();
                        t._end = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_END).ReadTime();
                        t._brightnessFactor = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_BRIGHTNESS_FACTOR).Read<float>(1.0f);
                        t._sightDistance = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SIGHT_DISTANCE).Read<int>(6000);

                        foreach (L3dVariable v in xmlEl.Elements(FileDescriptions.File_WEATHER_EL_TIME_RANGE_VARIABLE).Select(el =>
                        {
                            return new L3dVariable(undoManager)
                            {
                                Name = el.Attribute(FileDescriptions.File_WEATHER_AT_TIME_RANGE_VARIABLE_NAME).Read<string>("UnnamedVar"),
                                Value = el.Value
                            };
                        }))
                        {
                            t.Variables.Add(v);
                        }

                        XElement skybox = xmlEl.Element(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_SKYBOX);
                        if (skybox != null)
                        {
                            props = skybox.Element(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_SKYBOX_TEXTURES);
                            if (props != null)
                            {
                                t.TextureNorth = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH).Read<string>(string.Empty), ownerPath);
                                t.TextureEast = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST).Read<string>(string.Empty), ownerPath);
                                t.TextureSouth = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH).Read<string>(string.Empty), ownerPath);
                                t.TextureWest = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST).Read<string>(string.Empty), ownerPath);
                                t.TextureAbove = L3dFilePath.CreateRelativeToFile(props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE).Read<string>(string.Empty), ownerPath);

                                t.TextureNorthTileU = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH_TILE_U).Read<float>(0);
                                t.TextureNorthTileV = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH_TILE_V).Read<float>(0);
                                t.TextureEastTileU = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST_TILE_U).Read<float>(0);
                                t.TextureEastTileV = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST_TILE_V).Read<float>(0);
                                t.TextureSouthTileU = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH_TILE_U).Read<float>(0);
                                t.TextureSouthTileV = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH_TILE_V).Read<float>(0);
                                t.TextureWestTileU = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST_TILE_U).Read<float>(0);
                                t.TextureWestTileV = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST_TILE_V).Read<float>(0);
                                t.TextureAboveTileU = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE_TILE_U).Read<float>(0);
                                t.TextureAboveTileV = props.Attribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE_TILE_V).Read<float>(0);
                            }
                        }

                        XElement xmlPreci = xmlEl.Element(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_PRECIPITATION);
                        if (xmlPreci != null) 
                        {
                            t._snowfall = xmlPreci.Attribute(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_PRECIPITATION_TYPE).Read<int>(0) == 1;
                        }

                        return t;
                    }
                }
            }
            return null;
        }

        public XElement ConvertToXml(L3dFilePath ownerPath=null)
        {
            var elem = new XElement(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE,
                new XElement(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_PROPS,
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_BEGIN, Begin.TotalSeconds),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_END, End.TotalSeconds),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_BRIGHTNESS_FACTOR, BrightnessFactor),
                    new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SIGHT_DISTANCE, SightDistance)
                    ),
                    Variables.Select(v => new XElement(FileDescriptions.File_WEATHER_EL_TIME_RANGE_VARIABLE,
                        new XAttribute(FileDescriptions.File_WEATHER_AT_TIME_RANGE_VARIABLE_NAME, v.Name), v.Value)
                        ),
                    new XElement(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_SKYBOX,
                        new XElement(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_SKYBOX_TEXTURES,
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH, TextureNorth != null ? TextureNorth.GetPathRelativeToFile(ownerPath) : string.Empty),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST, TextureEast != null ? TextureEast.GetPathRelativeToFile(ownerPath) : string.Empty),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH, TextureSouth != null ? TextureSouth.GetPathRelativeToFile(ownerPath) : string.Empty),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST, TextureWest != null ? TextureWest.GetPathRelativeToFile(ownerPath) : string.Empty),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE, TextureNorth != null ? TextureAbove.GetPathRelativeToFile(ownerPath) : string.Empty),

                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH_TILE_U, TextureNorthTileU),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURENORTH_TILE_V, TextureNorthTileV),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST_TILE_U, TextureEastTileU),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREEAST_TILE_V, TextureEastTileV),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH_TILE_U, TextureSouthTileU),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTURESOUTH_TILE_V, TextureSouthTileV),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST_TILE_U, TextureWestTileU),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREWEST_TILE_V, TextureWestTileV),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE_TILE_U, TextureAboveTileU),
                            new XAttribute(FileDescriptions.FILE_WEATHER_AT_TIME_RANGE_SKYBOX_TEXTUREABOVE_TILE_V, TextureAboveTileV)                            
                            ))
                );
            if (_snowfall)
            {
                elem.Add(new XElement(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_PRECIPITATION, new XAttribute(FileDescriptions.FILE_WEATHER_EL_TIME_RANGE_PRECIPITATION_TYPE, 1)));
            }
            return elem;
        }
    }

    /// <summary>
    /// Representiert eine Loksim-Variable in einem Zeitbereich
    /// </summary>
    public class L3dVariable : UndoAwareFilePart
    {
        public L3dVariable(IUndoManager undoManager) : this(undoManager, string.Empty, string.Empty)
        {
        }

        public L3dVariable(IUndoManager undoManager, string name, string value)
            : base(undoManager)
        {
            _name = name;
            _value = value;
        }


        private string _name;

        /// <summary>
        /// Name der Variable
        /// </summary>
        public string Name
        {
            get { return _name; }
            set
            {
                if (value != _name)
                {
                    var oldVal = _name;
                    _name = value;
                    NotifyPropertyChanged("Name", () => Name = oldVal);
                }
            }
        }


        private string _value;

        /// <summary>
        /// Wert der Variable
        /// </summary>
        public string Value
        {
            get { return _value; }
            set
            {
                if (value != _value)
                {
                    var oldVal = _value;
                    _value = value;
                    NotifyPropertyChanged("Value", () => Value = oldVal);
                }
            }
        }
    }
}
