using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.FileWrappers;
using System.Windows;
using Loksim3D.WetterEdit.Views;

namespace Loksim3D.WetterEdit.ViewModels
{
    class DrivingCabViewModel : BaseL3dFileViewModel
    {
        private DrivingCabFile _drivingCabFile;

        public DrivingCabViewModel(DrivingCabFile drivingCabFile, Window parentWindow)
            : base(drivingCabFile, parentWindow)
        {
            _drivingCabFile = drivingCabFile;
        }

        protected override IEnumerable<Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogFilter> FileExtensionForSaveAs
        {
            get { throw new NotImplementedException(); }
        }

        

        public DrivingCabFile CabFile
        {
            get { return _drivingCabFile; }
            set
            {
                if (value != _drivingCabFile)
                {
                    var oldVal = _drivingCabFile;
                    _drivingCabFile = value;
                    NotifyPropertyChanged(() => CabFile);
                }
            }
        }

        public IEnumerable<Microsoft.WindowsAPICodePack.Dialogs.CommonFileDialogFilter> BitmapFileDlgFilters
        {
            get { return FileExtensions.TextureFiles; }
        }
    }
}
