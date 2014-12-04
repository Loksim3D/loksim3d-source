using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.FileWrappers;
using System.ComponentModel;
using System.Collections.ObjectModel;

namespace Loksim3D.WetterEdit.LogicWrappers
{
    /// <summary>
    /// Kapselt die Logik für die "Wettersteuerung"
    /// <br>Also Auswahl welche Textur zu einem definierten Zeitpunkt verwendet wird + Sichtweite und Helligkeit</br>
    /// </summary>
    public class WeatherCtrl : BaseModel
    {
        private WeatherFile _weatherFile;
        private TimeSpan _currentTime;

        private AmbientIllumination _currentIllumination;
        private ObservableCollection<WeatherTimeRange> _currentWeatherSet;

        public WeatherCtrl()
        {
            _currentSkyboxDir = FileWrappers.SkyBoxDirection.North;
            _tileTextureOneU = _tileTextureOneV = _tileTextureTwoU = _tileTextureTwoV = 1;
        }

        public WeatherFile WeatherFile
        {
            get { return _weatherFile; }
            set 
            {
                if (_weatherFile != value)
                {
                    if (_weatherFile != null)
                    {
                        _weatherFile.PropertyChanged -= _weatherFile_PropertyChanged;
                        _weatherFile.WeatherSet.CollectionChanged -= WeatherSet_CollectionChanged;
                        _weatherFile.Illumination.PropertyChanged -= Illumination_PropertyChanged;
                    }

                    _weatherFile = value;
                    CalcBrightness();
                    CalcTextures();
                    NotifyPropertyChanged("WeatherFile");

                    _weatherFile.PropertyChanged += new System.ComponentModel.PropertyChangedEventHandler(_weatherFile_PropertyChanged);
                    _weatherFile.WeatherSet.CollectionChanged += new System.Collections.Specialized.NotifyCollectionChangedEventHandler(WeatherSet_CollectionChanged);
                    _weatherFile.Illumination.PropertyChanged += new PropertyChangedEventHandler(Illumination_PropertyChanged);
                    _currentIllumination = _weatherFile.Illumination;
                    _currentWeatherSet = _weatherFile.WeatherSet;
                }
            }
        }

        void WeatherSet_CollectionChanged(object sender, System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
        {
            CalcBrightness();
            CalcTextures();
        }

        void Illumination_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            CalcBrightness();
        }

        void _weatherFile_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            switch (e.PropertyName)
            {
                case "Illumination":
                    if (_currentIllumination != null)
                    {
                        _currentIllumination.PropertyChanged -= Illumination_PropertyChanged;
                    }
                    _weatherFile.Illumination.PropertyChanged += new PropertyChangedEventHandler(Illumination_PropertyChanged);
                    _currentIllumination = _weatherFile.Illumination;
                    break;
                case "WeatherSet":
                    if (_currentWeatherSet != null)
                    {
                        _currentWeatherSet.CollectionChanged -= WeatherSet_CollectionChanged;
                    }
                    _weatherFile.WeatherSet.CollectionChanged += new System.Collections.Specialized.NotifyCollectionChangedEventHandler(WeatherSet_CollectionChanged);
                    _currentWeatherSet = _weatherFile.WeatherSet;
                    break;
            }
        }

        public TimeSpan CurrentTime
        {
            get { return _currentTime; }
            set
            {
                if (value != _currentTime)
                {
                    _currentTime = value;
                    Recalculate();
                    NotifyPropertyChanged("CurrentTime");
                }
            }
        }

        private SkyBoxDirection _currentSkyboxDir;

        public int CurrentSkyBoxDirection
        {
            get { return (int)_currentSkyboxDir; }
            set
            {
                SkyBoxDirection newVal = (SkyBoxDirection)value;
                if (newVal != _currentSkyboxDir)
                {
                    _currentSkyboxDir = newVal;
                    CalcTextures();
                    NotifyPropertyChanged("CurrentSkyBoxDirection");
                }
            }
        }

        public void Recalculate()
        {
            CalcBrightness();
            CalcTextures();
        }

        private float _brightness;

        public float Brightness 
        {
            get { return _brightness;  }
            private set
            {
                if (value != _brightness)
                {
                    _brightness = value;
                    NotifyPropertyChanged("Brightness");
                }
            }
        }
        
        private L3dFilePath _textureOne;

        public L3dFilePath TextureOne
        {
            get { return _textureOne; }
            private set
            {
                if (value != _textureOne)
                {
                    _textureOne = value;
                    NotifyPropertyChanged("TextureOne");
                }
            }
        }

        private L3dFilePath _textureTwo;

        public L3dFilePath TextureTwo
        {
            get { return _textureTwo; }
            private set
            {
                if (value != _textureTwo)
                {
                    _textureTwo = value;
                    NotifyPropertyChanged("TextureTwo");
                }
            }
        }

        private float _alphaValue;

        public float AlphaValue
        {
            get { return _alphaValue; }
            private set
            {
                if (value != _alphaValue)
                {
                    _alphaValue = value;
                    NotifyPropertyChanged("AlphaValue");
                }
            }
        }

        private float _sightDistance;

        public float SightDistance
        {
            get { return _sightDistance; }
            private set
            {
                if (value != _sightDistance)
                {
                    _sightDistance = value;
                    NotifyPropertyChanged("SightDistance");
                }
            }
        }

        private float _tileTextureOneU;

        public float TileTextureOneU
        {
            get { return _tileTextureOneU; }
            private set
            {
                if (value != _tileTextureOneU)
                {
                    _tileTextureOneU = value;
                    NotifyPropertyChanged("TileTextureOneU");
                }
            }
        }

        private float _tileTextureOneV;

        public float TileTextureOneV
        {
            get { return _tileTextureOneV; }
            private set
            {
                if (value != _tileTextureOneV)
                {
                    _tileTextureOneV = value;
                    NotifyPropertyChanged("TileTextureOneV");
                }
            }
        }

        private float _tileTextureTwoU;

        public float TileTextureTwoU
        {
            get { return _tileTextureTwoU; }
            private set
            {
                if (value != _tileTextureTwoU)
                {
                    _tileTextureTwoU = value;
                    NotifyPropertyChanged("TileTextureTwoU");
                }
            }
        }

        private float _tileTextureTwoV;

        public float TileTextureTwoV
        {
            get { return _tileTextureTwoV; }
            private set
            {
                if (value != _tileTextureTwoV)
                {
                    _tileTextureTwoV = value;
                    NotifyPropertyChanged("TileTextureTwoV");
                }
            }
        }

        /// <summary>
        ///  Prueft ob die uebergebene zeit zwischen start und end liegt (Zeit in Sekunden)
        /// </summary>
        private bool IsTimeBetween(double start, double end, double time)
        {
            if (time > start && time < end)
                return true;
            if (end < start)
                return time < end || time > start;
            return false;
        }

        private void CalcBrightness()
        {
            double ActTime = CurrentTime.TotalSeconds;

            double startHellwerden = _weatherFile.Illumination.DawnBegin.TotalSeconds;
            double endeHellwerden = _weatherFile.Illumination.DawnEnd.TotalSeconds;

            double startDunkelwerden = _weatherFile.Illumination.DuskBegin.TotalSeconds;
            double endeDunkelwerden = _weatherFile.Illumination.DuskEnd.TotalSeconds;

            float helligkeitNacht = _weatherFile.Illumination.BrightnessNight;
            float helligkeitTag = _weatherFile.Illumination.BrightnessDay;
            float helligkeitDiff = helligkeitTag - helligkeitNacht;

            // Funktioniert nur, falls keine Dämmerung über Mitternacht auftritt!!
            // Wirklich? Muss ich mir nochmal anschauen; vl funktioniert es auch über mitternacht
            if (IsTimeBetween(endeHellwerden, startDunkelwerden, ActTime))
            {
                //hellster Tag
                Brightness = helligkeitTag;
            }
            else if (IsTimeBetween(startDunkelwerden, endeDunkelwerden, ActTime))
            {
                //es wird langsam dunkler ("Sonnenuntergang")
                ActTime -= startDunkelwerden;
                Brightness = (float)Math.Max(helligkeitNacht, Math.Min(helligkeitTag, helligkeitTag - (ActTime / (endeDunkelwerden - startDunkelwerden)) * helligkeitDiff));
            }
            else if (IsTimeBetween(endeDunkelwerden, startHellwerden, ActTime))
            {
                //Nacht
                Brightness = helligkeitNacht;
            }
            else
            {
                //es wird heller ("Sonnenaufgang")
                ActTime -= startHellwerden;
                Brightness = (float)Math.Max(helligkeitNacht, Math.Min(helligkeitTag, (ActTime / (endeHellwerden - startHellwerden)) * helligkeitDiff));
            }
        }

        private void CalcTextures()
        {
            double ActTime = _currentTime.TotalSeconds;

            AlphaValue = 0.0f;
            int indTextureOne = -1;
            int indTextureTwo = -1;
            float brightnessFactor = 1.0f;
            
            var skyEntries = _weatherFile.WeatherSet.OrderBy(w =>  w.Begin).ToList();
            for (int i = 0; i < skyEntries.Count; ++i)
            {
                if (IsTimeBetween(skyEntries[i].Begin.TotalSeconds, skyEntries[i].End.TotalSeconds, ActTime))
                {
                    indTextureOne = i;
                    int nextInd = (i + 1) % skyEntries.Count;

                    double startOne = skyEntries[i].Begin.TotalSeconds;
                    double endOne = skyEntries[i].End.TotalSeconds;

                    double startTwo = skyEntries[nextInd].Begin.TotalSeconds;
                    double endTwo = skyEntries[nextInd].End.TotalSeconds;

                    if (!IsTimeBetween(startTwo, endTwo, ActTime))
                    {
                        nextInd = skyEntries.Count - 1;
                        if (i == 0 && skyEntries.Count > 1 && IsTimeBetween(skyEntries[nextInd].Begin.TotalSeconds, skyEntries[nextInd].End.TotalSeconds, ActTime))
                        {
                            // letzte und erste Tex überblenden
                            indTextureOne = nextInd;
                            startOne = skyEntries[nextInd].Begin.TotalSeconds;
                            endOne = skyEntries[nextInd].End.TotalSeconds;

                            nextInd = i;
                            startTwo = skyEntries[i].Begin.TotalSeconds;
                            endTwo = skyEntries[i].End.TotalSeconds;
                        }
                        else
                        {
                            //nur Textur 1
                            AlphaValue = 0.0f;
                            SightDistance = skyEntries[indTextureOne].SightDistance;
                            brightnessFactor = skyEntries[indTextureOne].BrightnessFactor;
                            break;
                        }
                    }
                    if (IsTimeBetween(startTwo, endOne, ActTime))
                    {
                        // Zwischen Anfang von 1 und Ende von 2
                        double diff = endOne - startTwo;
                        if (diff < 0)
                        {
                            diff += 24 * 3600;
                        }
                        double div = ActTime - startTwo;
                        if (div < 0)
                        {
                            div += 24 * 3600;
                        }
                        AlphaValue = (float)(div / diff);
                        indTextureTwo = nextInd;
                    }
                    else
                    {
                        // Zwischen Anfang von 2 und Ende von 1
                        double diff = endTwo - startOne;

                        if (diff < 0)
                        {
                            diff += 24 * 3600;
                        }
                        double div = ActTime - startOne;
                        if (div < 0)
                        {
                            div += 24 * 3600;
                        }
                        AlphaValue = (float)(1.0f - div / diff);
                        indTextureTwo = nextInd;
                    }

                    int nebelOne = skyEntries[indTextureOne].SightDistance;
                    int nebelTwo = skyEntries[indTextureTwo].SightDistance;
                    if (nebelOne == 0 && nebelTwo != 0)
                    {
                        nebelOne = 10000;
                    }
                    if (nebelTwo == 0 && nebelOne != 0)
                    {
                        nebelTwo = 10000;
                    }
                    SightDistance = (int)(nebelOne +
                        (nebelTwo - nebelOne) * AlphaValue);
                    brightnessFactor = skyEntries[indTextureOne].BrightnessFactor +
                        (skyEntries[indTextureTwo].BrightnessFactor - skyEntries[indTextureOne].BrightnessFactor) * AlphaValue;
                    break;
                }
            }
            Brightness *= brightnessFactor;
            if (indTextureOne != -1)
            {
                TileTextureOneU = skyEntries[indTextureOne].GetSkyboxTileU(_currentSkyboxDir);
                TileTextureOneV = skyEntries[indTextureOne].GetSkyboxTileV(_currentSkyboxDir);
                TextureOne = skyEntries[indTextureOne].GetSkyboxTexture(_currentSkyboxDir);
            }
            if (indTextureTwo != -1)
            {
                TileTextureTwoU = skyEntries[indTextureTwo].GetSkyboxTileU(_currentSkyboxDir);
                TileTextureTwoV = skyEntries[indTextureTwo].GetSkyboxTileV(_currentSkyboxDir);
                TextureTwo = skyEntries[indTextureTwo].GetSkyboxTexture(_currentSkyboxDir);
            }
        }
    }
}
