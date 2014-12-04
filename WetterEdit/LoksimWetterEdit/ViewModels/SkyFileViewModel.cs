using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.FileWrappers;
using System.Windows;
using System.Windows.Input;
using Microsoft.WindowsAPICodePack.Dialogs;
using Loksim3D.WetterEdit.Views;
using Loksim3D.WetterEdit.ViewModels.Helpers;
using System.Media;

namespace Loksim3D.WetterEdit.ViewModels
{
    /// <summary>
    /// ViewModel für Sky-Dateien
    /// </summary>
    public class SkyFileViewModel : BaseL3dFileViewModel
    {
        private SkyFile _skyFile;
        private Weather _selectedWeather;
        private WeatherFile _currentWeather;

        private static Guid WEATHER_DLG_GUID = new Guid((int)0x909b1ec, 0x648b, 0x4b99, new byte[] { 0xb6, 0x7c, 0xa0, 0xd6, 0xd4, 0x0d, 0xbc, 0xc4 } );

        public SkyFileViewModel(SkyFile skyFile, Window parentWindow)
            : base(skyFile, parentWindow)
        {
            _skyFile = skyFile;
            _selectedWeather = skyFile.WeatherSets.FirstOrDefault();

            RemoveFileCmd = new RelayCommand(obj =>
                {
                    _skyFile.WeatherSets.Remove(_selectedWeather);
                    SelectedWeatherFile = null;
                },
                obj =>
                {
                    return SelectedWeatherFile != null;
                });

            AddFileCmd = new RelayCommand(obj =>
                {
                    string s = DialogHelpers.OpenLoksimFile(string.Empty, FileExtensions.WeatherFiles, parentWindow, WEATHER_DLG_GUID);
                    if (!string.IsNullOrEmpty(s))
                    {
                        Weather w = new Weather(SkyFile, new L3dFilePath(s));
                        _skyFile.WeatherSets.Add(w);
                        SelectedWeatherFile = w;
                    }
                });

            CopyWeatherFile = new RelayCommand(arg =>
            {
                if (SelectedWeatherFile != null)
                {
                    try
                    {
                        Clipboard.SetText(SelectedWeatherFile.ConvertToXml().ToString(), TextDataFormat.UnicodeText);
                    }
                    catch (Exception)
                    {
                        SystemSounds.Exclamation.Play();
                    }
                }
            },
                arg =>
                {
                    return SelectedWeatherFile != null;
                }
            );

            CutWeatherFile = new RelayCommand(arg =>
            {
                if (SelectedWeatherFile != null)
                {
                    try
                    {
                        Clipboard.SetText(SelectedWeatherFile.ConvertToXml().ToString(), TextDataFormat.UnicodeText);
                        SkyFile.WeatherSets.Remove(SelectedWeatherFile);
                    }
                    catch (Exception)
                    {
                        SystemSounds.Exclamation.Play();
                    }
                }
            },
                arg =>
                {
                    return SelectedWeatherFile != null;
                }
            );

            PasteWeatherFile = new RelayCommand(arg =>
            {
                var w = Weather.ReadFromXml(Clipboard.GetText(), SkyFile);
                if (w != null)
                {
                    SkyFile.WeatherSets.Add(w);
                    SelectedWeatherFile = w;
                }
            },
                arg =>
                {
                    return Clipboard.ContainsText() && Weather.ReadFromXml(Clipboard.GetText(), SkyFile) != null;
                }
            );

            WeatherFileDown = new RelayCommand(arg =>
            {
                int ind = SelectedWeatherFileIndex;
                if (ind >= 0 && ind < SkyFile.WeatherSets.Count - 1)
                {
                    var w = SkyFile.WeatherSets[ind];
                    SkyFile.WeatherSets.RemoveAt(ind);
                    SkyFile.WeatherSets.Insert(ind + 1, w);
                }
            },
                arg =>
                {
                    return SelectedWeatherFileIndex >= 0 && SelectedWeatherFileIndex < SkyFile.WeatherSets.Count - 1;
                }
            );

            WeatherFileUp = new RelayCommand(arg =>
            {
                int ind = SelectedWeatherFileIndex;
                if (ind > 0 && ind < SkyFile.WeatherSets.Count)
                {
                    var w = SkyFile.WeatherSets[ind];
                    SkyFile.WeatherSets.RemoveAt(ind);
                    SkyFile.WeatherSets.Insert(ind - 1, w);
                }
            },
                arg =>
                {
                    return SelectedWeatherFileIndex > 0 && SelectedWeatherFileIndex < SkyFile.WeatherSets.Count;
                }
            );

        }

        public SkyFile SkyFile
        {
            get { return _skyFile;  }
        }

        protected override IEnumerable<Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogFilter> FileExtensionForSaveAs
        {
            get { return FileExtensions.SkyFiles; }
        }

        public IEnumerable<CommonFileDialogFilter> FileDlgWeatherFilesFilters
        {
            get { return FileExtensions.WeatherFiles; }
        }

        public Guid FileDlgWeatherFilesGuid
        {
            get { return WEATHER_DLG_GUID; }
        }

        public Weather SelectedWeatherFile
        {
            get { return _selectedWeather; }
            set
            {
                if (value != _selectedWeather)
                {
                    _selectedWeather = value;
                    NotifyPropertyChanged("SelectedWeatherFile");
                    if (_selectedWeather != null)
                    {
                        try
                        {
                            WeatherFile w = new WeatherFile();
                            w.LoadFromFile(_selectedWeather.WeatherFile);
                            CurrentWeather = w;
                        }
                        catch (Exception)
                        {
                            CurrentWeather = null;
                        }
                    }
                    else
                    {
                        CurrentWeather = null;
                    }
                }
            }
        }

        public WeatherFile CurrentWeather
        {
            get { return _currentWeather; }
            private set
            {
                if (value != _currentWeather)
                {
                    _currentWeather = value;
                    NotifyPropertyChanged("CurrentWeather");
                }
            }
        }

        public int SelectedWeatherFileIndex { get; set; }

        public ICommand RemoveFileCmd { private set; get; }

        public ICommand AddFileCmd { private set; get; }

        public ICommand CutWeatherFile { private set; get; }

        public ICommand PasteWeatherFile { private set; get; }

        public ICommand CopyWeatherFile { private set; get; }

        public ICommand WeatherFileDown { private set; get; }

        public ICommand WeatherFileUp { private set; get; }
    }
}
