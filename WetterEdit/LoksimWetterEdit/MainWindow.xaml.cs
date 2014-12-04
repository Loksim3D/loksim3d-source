using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Loksim3D.WetterEdit.FileWrappers;
using Loksim3D.WetterEdit.Views;
using System.Reflection;
using System.Diagnostics;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Data;
using Loksim3D.WetterEdit.ViewModels;
using Loksim3D.WetterEdit.Views.Dialogs;

namespace Loksim3D.WetterEdit
{
    /// <summary>
    /// Interaction logic for MainWindow.o
    /// </summary>
    public partial class MainWindow : Window
    {
        private BaseL3dFileViewModel _currentViewModel;
        private string _fileToOpen;

        private static List<MainWindow> _openAppWindows;

        private static Guid MAIN_DLG_OPEN = new Guid((int)0x159e1ec, 0x648b, 0x4b73, new byte[] { 0xb6, 0x7c, 0xa0, 0xd6, 0xd8, 0x2d, 0xbc, 0x84 });

        public MainWindow()
        {
            InitializeComponent();

            if (_openAppWindows == null)
            {
                _openAppWindows = new List<MainWindow>();
            }
            _openAppWindows.Add(this);         
        }


        public MainWindow(string fileToOpen) : this()
        {
            _fileToOpen = fileToOpen;
        }


        internal void CreateNewFile(string fileToOpen)
        {
            if (fileToOpen != null)
            {
                string arg0 = _fileToOpen.Trim();
                LoksimFile file = null;
                switch (arg0.ToLower())
                {
                    case "-l3dsky":
                        file = new SkyFile();
                        break;
                    case "-l3dwth":
                        file = WeatherFile.CreateNewDefaultFile();
                        break;
                    case "-l3dfst":
                        file = new DrivingCabFile();
                        break;
                    default:
                        file = LoadFile(new L3dFilePath(arg0));
                        break;
                }
                if (file != null)
                {
                    SetCtrlAndViewModel(file);
                }
            }
#if DEBUG
            //TODO Just for driving cab dev
            /*
            else
            {
                LoksimFile f = new DrivingCabFile();
                SetCtrlAndViewModel(f);
            }
             */
#endif
    
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            CreateNewFile(_fileToOpen);
            SetRecentFilesMenu();     
        }

        void SetRecentFilesMenu()
        {
            int insertInd = menuFile.Items.IndexOf(separatorBeforeFiles) + 1;
            for (int i = insertInd; i < menuFile.Items.Count; i++)
            {
                if (menuFile.Items[i] is Separator)
                {
                    break;
                }
                menuFile.Items.RemoveAt(i);
                i--;
            }

            foreach (L3dFilePath p in Settings.RegistrySettings.Default.RecentFiles)
            {
                MenuItem it = new MenuItem { Header = p.Filename };
                it.Tag = p;
                it.Click += new RoutedEventHandler(recentFileMenuItem_Click);
                menuFile.Items.Insert(insertInd, it);
                insertInd++;
            }
        }

        void recentFileMenuItem_Click(object sender, RoutedEventArgs e)
        {
            MenuItem m = sender as MenuItem;
            if (m != null && m.Tag != null)
            {
                OpenFile(m.Tag as L3dFilePath);
            }
        }

        private LoksimFile LoadFile(L3dFilePath path)
        {
            LoksimFile file = null;
            try
            {
                switch (System.IO.Path.GetExtension(path.AbsolutePath).ToLower())
                {
                    case ".l3dwth":
                        {
                            WeatherFile f = new WeatherFile();
                            f.LoadFromFile(path);
                            file = f;
                        }
                        break;
                    case ".l3dsky":
                        {
                            SkyFile f = new SkyFile();
                            f.LoadFromFile(path);
                            file = f;
                        }
                        break;
                    case ".l3dfst":
                        {
                            DrivingCabFile f = new DrivingCabFile();
                            f.LoadFromFile(path);
                            file = f;
                        }
                        break;
                    default:
                        Debug.Assert(false, "Unknown File Type");
                        break;
                }
                this.Title = Loksim3D.WetterEdit.Resources.Strings.AppName + " [" + path.Filename + "]";
                Settings.RegistrySettings.Default.AddRecentFile(path);
            }
            catch (Exception ex)
            {
                Mouse.OverrideCursor = null;
                MessageBox.Show(String.Format(Loksim3D.WetterEdit.Resources.Strings.ErrorLoadingFile_0_Msg_1_, path.Filename, ex.Message),
                    Loksim3D.WetterEdit.Resources.Strings.AppName, MessageBoxButton.OK, MessageBoxImage.Error);
            }
            return file;
        }

        private void SetCtrlAndViewModel(LoksimFile file)
        {
            UserControl ctrl = null;
            if (file is WeatherFile)
            {
                _currentViewModel = new WeatherViewModel(file as WeatherFile, this);
                ctrl = new WeatherFileEdit();
            }
            else if (file is SkyFile)
            {
                _currentViewModel = new SkyFileViewModel(file as SkyFile, this);
                ctrl = new SkyFileEdit();
            }
            else if (file is DrivingCabFile)
            {
                _currentViewModel = new DrivingCabViewModel(file as DrivingCabFile, this);
                ctrl = new DrivingCabFileEdit();
            }
            if (ctrl != null)
            {
                ctrl.DataContext = _currentViewModel;
                mainDock.Children.Add(ctrl);
            }
        }

        private void CommandNew_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }
        private void CommandNew_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            NewFileWindow dlg = new NewFileWindow();
            dlg.Owner = this;
            if (dlg.ShowDialog().GetValueOrDefault(false))
            {
                LoksimFile file = dlg.SelectedFile;
                if (_currentViewModel != null)
                {
                    if (file is WeatherFile)
                    {
                        new MainWindow("-l3dwth").Show();
                    }
                    else if (file is SkyFile)
                    {
                        new MainWindow("-l3dsky").Show();
                    }
                    else if (file is DrivingCabFile)
                    {
                        new MainWindow("-l3dfst").Show();
                    }
                    else
                    {
                        Debug.Assert(false, "Unknown File Type");
                    }
                }
                else
                {
                    SetCtrlAndViewModel(file);
                }
            }
        }

        private void CommandOpen_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        public void OpenFile(L3dFilePath p)
        {
            if (!L3dFilePath.IsNullOrEmpty(p))
            {
                var w = _openAppWindows.FirstOrDefault(m => m._currentViewModel != null && m._currentViewModel.CurrentFile != null &&
                    L3dFilePath.Equals(m._currentViewModel.CurrentFile.OwnPath, p));
                if (w != null)
                {
                    //w.BringIntoView();
                    if (w.WindowState == System.Windows.WindowState.Minimized)
                    {
                        w.WindowState = System.Windows.WindowState.Normal;
                    }
                    w.Activate();
                }
                else
                {
                    if (_currentViewModel == null)
                    {
                        Mouse.OverrideCursor = Cursors.Wait;
                        LoksimFile file = LoadFile(p);
                        SetCtrlAndViewModel(file);
                        Mouse.OverrideCursor = null;
                    }
                    else
                    {
                        new MainWindow(p.AbsolutePath).Show();
                    }
                }
                SetRecentFilesMenu();
            }
        }

        private void CommandOpen_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            string str = DialogHelpers.OpenLoksimFile(string.Empty, FileExtensions.AllLoksimFiles, this, MAIN_DLG_OPEN);
            if (!string.IsNullOrEmpty(str))
            {
                L3dFilePath p = new L3dFilePath(str);
                if (!L3dFilePath.IsNullOrEmpty(p))
                {
                    OpenFile(p);
                }
            }
        }

        private void CommandSave_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                e.CanExecute = _currentViewModel.FileSaveCmd.CanExecute(null);
                e.Handled = true;
            }
        }
        private void CommandSave_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                _currentViewModel.FileSaveCmd.Execute(null);
                SetRecentFilesMenu();
            }
        }

        private void CommandSaveAs_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                e.CanExecute = _currentViewModel.FileSaveAsCmd.CanExecute(null);
                e.Handled = true;
            }
        }
        private void CommandSaveAs_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                _currentViewModel.FileSaveAsCmd.Execute(null);
                if (!L3dFilePath.IsNullOrEmpty(_currentViewModel.CurrentFile.OwnPath))
                {
                    Settings.RegistrySettings.Default.AddRecentFile(_currentViewModel.CurrentFile.OwnPath);
                    SetRecentFilesMenu();
                    this.Title = Loksim3D.WetterEdit.Resources.Strings.AppName + " [" + _currentViewModel.CurrentFile.OwnPath.Filename + "]";
                }
            }
        }

        private void CommandProperties_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                e.CanExecute = _currentViewModel.FilePropertiesCmd.CanExecute(e);
            }
            else
            {
                e.CanExecute = false;
            }
        }

        private void CommandProperties_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                _currentViewModel.FilePropertiesCmd.Execute(e);
            }
        }

        private void CommandClose_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }
        private void CommandClose_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            this.Close();
        }

        private void CommandUndo_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _currentViewModel != null && _currentViewModel.UndoCmd.CanExecute(null);
        }
        private void CommandUndo_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                _currentViewModel.UndoCmd.Execute(e);
            }
        }

        private void CommandRedo_CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = _currentViewModel != null && _currentViewModel.RedoCmd.CanExecute(null);
        }
        private void CommandRedo_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (_currentViewModel != null)
            {
                _currentViewModel.RedoCmd.Execute(e);
            }
        }

        private void MenuItemHelpAbout_Click(object sender, RoutedEventArgs e)
        {
            AboutWindow wnd = new AboutWindow();
            wnd.Owner = this;
            wnd.ShowDialog();
        }

        private void MenuItemEditSettings_Click(object sender, RoutedEventArgs e)
        {
            SettingsWindow wnd = new SettingsWindow();
            wnd.Owner = this;
            wnd.ShowDialog();
        }

        private void CommandHelp_Executed(object sender, ExecutedRoutedEventArgs e)
        {
            Process.Start(@"http://www.loksim.de/Wiki/index.php/LoksimWetterEdit");
        }

        private void Window_Closing(object sender, CancelEventArgs e)
        {
            e.Cancel = _currentViewModel != null && !_currentViewModel.CloseApplication();
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            _openAppWindows.Remove(this);
        }

        private void MenuItem_Click(object sender, RoutedEventArgs e)
        {

        }

    }
}
