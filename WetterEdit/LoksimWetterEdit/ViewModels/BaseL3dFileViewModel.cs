using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Reflection;
using System.Diagnostics;
using Loksim3D.WetterEdit.FileWrappers;
using System.Windows.Input;
using Microsoft.WindowsAPICodePack.Dialogs;
using System.Windows;
using Loksim3D.WetterEdit.Views;
using Loksim3D.WetterEdit.ViewModels.Helpers;
using System.Globalization;

namespace Loksim3D.WetterEdit.ViewModels
{
    /// <summary>
    /// ViewModel Basisklasse für alle ViewModels die eine komplette Datei "kapseln"
    /// </summary>
    public abstract class BaseL3dFileViewModel : BaseModel
    {
        private ICommand _fileSaveCmd;
        private ICommand _fileSaveAsCmd;

        private static Guid SAVEAS_DLG_GUID = new Guid((int)0x812b1ec, 0x648b, 0x4b99, new byte[] { 0xb6, 0x7c, 0xa0, 0xd6, 0xd8, 0x2d, 0xbc, 0x84 });

        protected BaseL3dFileViewModel(LoksimFile openFile, Window parentWindow)
        {            
            CurrentFile = openFile;
            ParentWindow = parentWindow;

            _fileSaveCmd = new RelayCommand(obj =>
                {
                    SaveOrSaveAs(false);
                },
                obj => 
                {
                    return CurrentFile != null && !L3dFilePath.IsNullOrEmpty(CurrentFile.OwnPath);
                }
            );


            _fileSaveAsCmd = new RelayCommand(obj =>
                {
                    SaveOrSaveAs(true);
                },
                obj =>
                {
                    return CurrentFile != null;
                }
            );

            FilePropertiesCmd = new RelayCommand(obj =>
                {
                    Window dlg = new FilePropertiesWindow(CurrentFile);
                    dlg.Owner = ParentWindow;
                    dlg.ShowDialog();
                },
                obj =>
                {
                    return CurrentFile != null;
                }
            );

            UndoCmd = new RelayCommand(obj =>
                {
                    UndoAwareFile f = CurrentFile as UndoAwareFile;
                    if (f != null)
                    {
                        f.Undo();
                    }
                },
                obj =>
                {
                    UndoAwareFile f = CurrentFile as UndoAwareFile;
                    return f != null && f.UndoPossible;
                }
            );

            RedoCmd = new RelayCommand(obj =>
                {
                    UndoAwareFile f = CurrentFile as UndoAwareFile;
                    if (f != null)
                    {
                        f.Redo();
                    }
                },
                obj =>
                {
                    UndoAwareFile f = CurrentFile as UndoAwareFile;
                    return f != null && f.RedoPossible;
                }
            );
        }

        private bool SaveOrSaveAs(bool saveAs)
        {
            L3dFilePath p;
            if (saveAs || L3dFilePath.IsNullOrEmpty(CurrentFile.OwnPath))
            {
                p = DialogHelpers.SaveLoksimFile(CurrentFile.OwnPath, FileExtensionForSaveAs, SAVEAS_DLG_GUID);
            }
            else
            {
                p = CurrentFile.OwnPath;
            }

            try
            {
                if (p != null)
                {
                    CurrentFile.SaveToFile(p);
                    return true;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(String.Format(CultureInfo.CurrentCulture, Resources.Strings.FileSaveError, CurrentFile.OwnPath, ex.Message),
                    Resources.Strings.AppName, System.Windows.MessageBoxButton.OK, System.Windows.MessageBoxImage.Error);
            }
            return false;
        }

        public LoksimFile CurrentFile { get; set; }

        /// <summary>
        /// Liefert die Dateiendung(en) zurück welche beim Speichern der Datei verwendet werden können
        /// </summary>
        protected abstract IEnumerable<CommonFileDialogFilter> FileExtensionForSaveAs
        {
            get;
        }

        public bool CloseApplication()
        {
            if (CurrentFile == null || !CurrentFile.WasModifiedSinceLastSave())
            {
                return true;
            }

            var res = MessageBox.Show(Resources.Strings.AppCloseUnsavedChanges, Resources.Strings.AppName,
                System.Windows.MessageBoxButton.YesNoCancel, System.Windows.MessageBoxImage.Question);
            switch (res)
            {
                case System.Windows.MessageBoxResult.Cancel:
                    return false;
                case System.Windows.MessageBoxResult.No:
                    return true;
                case System.Windows.MessageBoxResult.Yes:                    
                    return SaveOrSaveAs(false);
            }
            Debug.Assert(false, "Unexpected MessageBox Result");
            return false;
        }

        public ICommand FileSaveCmd
        {
            get { return _fileSaveCmd; }
            protected set  { _fileSaveCmd = value; }
        }

        public ICommand FileSaveAsCmd
        {
            get { return _fileSaveAsCmd; }
            protected set { _fileSaveAsCmd = value; }
        }

        public ICommand FilePropertiesCmd { get; protected set; }

        public ICommand UndoCmd { get; protected set; }

        public ICommand RedoCmd { get; protected set; }

        protected Window ParentWindow { get; private set; }
    }
}
