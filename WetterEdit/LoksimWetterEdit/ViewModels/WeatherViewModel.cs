using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.FileWrappers;
using System.Windows.Input;
using Microsoft.Win32;
using System.Collections.ObjectModel;
using System.ComponentModel;
using Loksim3D.WetterEdit.LogicWrappers;
using System.Windows;
using Microsoft.WindowsAPICodePack.Dialogs;
using Loksim3D.WetterEdit.Views;
using Loksim3D.WetterEdit.ViewModels.Helpers;
using System.Windows.Data;
using System.Media;

namespace Loksim3D.WetterEdit.ViewModels
{
    /// <summary>
    /// ViewModel für Wetterdateien
    /// </summary>
    public class WeatherViewModel  : BaseL3dFileViewModel
    {
        private static Guid TEXTURE_DLG_GUID = new Guid((int)0x909b1ec, 0x648b, 0x4b99, new byte[] { 0xb6, 0x7c, 0xa0, 0xd6, 0xd8, 0x2d, 0xbc, 0x84 });

        private WeatherFile _weatherFile;
        private WeatherCtrl _weatherCtrl;
        private CollectionViewSource _timerangesView;
        private Boolean _timerangesSortAsc;

        private static List<string> _predefinedVariables = new List<string> { "Gewitter", "Nebel", "Regen", "Sonne", "Wolken" };

        public WeatherViewModel(WeatherFile weatherFile, Window parentWindow)
           : base(weatherFile, parentWindow)
        {
            _weatherFile = weatherFile;

            NewTimeRangeCmd = new RelayCommand(arg =>
                {
                    _weatherFile.WeatherSet.Add(new WeatherTimeRange(WeatherFile));
                    SelectedTimeRange = _weatherFile.WeatherSet.Last();
                }
            );

            DelTimeRangeCmd = new RelayCommand(arg =>
                {
                    _weatherFile.WeatherSet.Remove(SelectedTimeRange);
                },
                arg =>
                {
                    return SelectedTimeRange != null;
                }
            );

            AddVariableCmd = new RelayCommand(arg =>
                {
                    if (SelectedTimeRange != null)
                    {
                        SelectedTimeRange.Variables.Add(new L3dVariable(WeatherFile, "Variable", string.Empty));
                        SelectedVariable = SelectedTimeRange.Variables.Last();
                    }
                },
                arg =>
                {
                    return SelectedTimeRange != null;
                }
            );

            RemoveVariableCmd = new RelayCommand(arg =>
                {
                    if (SelectedTimeRange != null && SelectedVariable != null)
                    {
                        SelectedTimeRange.Variables.Remove(SelectedVariable);
                        SelectedVariable = null;
                    }
                },
                arg =>
                {
                    return SelectedTimeRange != null && SelectedVariable != null;
                }
            );

            CopyTimeRange = new RelayCommand(arg =>
                {
                    try
                    {
                        if (SelectedTimeRange != null)
                        {
                            Clipboard.SetText(SelectedTimeRange.ConvertToXml().ToString(), TextDataFormat.UnicodeText);
                        }
                    }
                    catch (Exception)
                    {
                        SystemSounds.Exclamation.Play();
                    }
                },
                arg =>
                {
                    return SelectedTimeRange != null;
                }
            );

            CutTimeRange = new RelayCommand(arg =>
                {
                    try
                    {
                        if (SelectedTimeRange != null)
                        {
                            Clipboard.SetText(SelectedTimeRange.ConvertToXml().ToString(), TextDataFormat.UnicodeText);
                            _weatherFile.WeatherSet.Remove(SelectedTimeRange);
                        }
                    }
                    catch (Exception)
                    {
                        SystemSounds.Exclamation.Play();
                    }
                },
                arg =>
                {
                    return SelectedTimeRange != null;
                }
            );

            PasteTimeRange = new RelayCommand(arg =>
                {
                    var w = WeatherTimeRange.ReadFromXml(Clipboard.GetText(), WeatherFile);
                    if (w != null)
                    {
                        _weatherFile.WeatherSet.Add(w);
                        SelectedTimeRange = w;
                    }
                },
                arg =>
                {
                    try
                    {
                        return Clipboard.ContainsText() && WeatherTimeRange.ReadFromXml(Clipboard.GetText(), WeatherFile) != null;
                    }
                    catch (Exception)
                    {
                        return false;
                    }
                }
            );

            SortTimeRangesCmd = new RelayCommand(arg =>
                {
                    string sortCol = (string)arg;
                    _timerangesView.SortDescriptions.Clear();

                    if (_timerangesSortAsc)
                    {
                        _timerangesView.SortDescriptions.Add(new SortDescription(sortCol, ListSortDirection.Ascending));
                    }
                    else
                    {
                        _timerangesView.SortDescriptions.Add(new SortDescription(sortCol, ListSortDirection.Descending));
                    }
                    _timerangesSortAsc = !_timerangesSortAsc;
                }
                );

            _weatherCtrl = new WeatherCtrl();
            _weatherCtrl.WeatherFile = _weatherFile;

            _timerangesView = new CollectionViewSource();
            _timerangesView.Source = _weatherFile.WeatherSet;

            SelectedTimeRange = _weatherFile.WeatherSet.FirstOrDefault();
        }

        public WeatherCtrl WeatherTest
        {
            get
            {
                return _weatherCtrl;
            }
        }


        public WeatherFile WeatherFile
        {
            get { return _weatherFile; }
        }

        public IEnumerable<CommonFileDialogFilter> FileDlgTextureFilters
        {
            get { return FileExtensions.TextureFiles; }
        }

        public Guid FileDlgTextureGuid
        {
            get { return TEXTURE_DLG_GUID; }
        }

        public ICommand NewTimeRangeCmd { get; private set; }

        public ICommand DelTimeRangeCmd { get; private set; }

        public ICommand AddVariableCmd { get; private set; }

        public ICommand RemoveVariableCmd { get; private set; }

        public ICommand CopyTimeRange { get; private set; }

        public ICommand CutTimeRange { get; private set; }

        public ICommand PasteTimeRange { get; private set; }

        public ICommand SortTimeRangesCmd { get; private set; }


        protected override IEnumerable<Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogFilter> FileExtensionForSaveAs
        {
            get { return FileExtensions.WeatherFiles; }
        }

        
        private L3dVariable _selectedVariable;

        public L3dVariable SelectedVariable
        {
            get { return _selectedVariable; }
            set
            {
                if (value != _selectedVariable)
                {
                    _selectedVariable = value;
                    NotifyPropertyChanged("SelectedVariable");
                }
            }
        }

        
        private WeatherTimeRange _selectedTimeRange;

        public WeatherTimeRange SelectedTimeRange
        {
            get { return _selectedTimeRange; }
            set
            {
                if (value != _selectedTimeRange)
                {
                    _selectedTimeRange = value;
                    NotifyPropertyChanged("SelectedTimeRange");
                    if (value != null && value.Begin != value.End)
                    {
                        _weatherCtrl.CurrentTime = value.Begin.Add(new TimeSpan(0, 0, (int)value.Length.TotalSeconds / 2));
                    }
                }
            }
        }

        public ListCollectionView TimeRangesView
        {
            get { return (ListCollectionView)_timerangesView.View; }
        }

        public IList<string> PredefinedVariables { get { return _predefinedVariables; } }
    }
}
