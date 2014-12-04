using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using Microsoft.WindowsAPICodePack.Dialogs;
using System.IO;

namespace Loksim3D.WetterEdit.Views.Controls
{
    /// <summary>
    /// Control welches einen L3dFilePath in TextBox anzeigt + Button zum Suchen einer Datei
    /// <br>In TextBox kann ein Pfad eingegeben werden, dies wird mit AutoComplete unterstützt. Ein Klick auf Button öffnen "Datei-Öffnen" Dialog und ermöglicht
    /// Auswahl eines Pfads</br>
    /// </summary>
    public partial class FilePathControl : UserControl
    {
        private IEnumerable<string> _possibleExtensions;
        private L3dFilePath _curPath;

        public FilePathControl()
        {
            _possibleExtensions = Enumerable.Empty<string>();
            InitializeComponent();
            tbPath.Populating += new PopulatingEventHandler(tbPath_Populating);
            tbPath.FilterMode = AutoCompleteFilterMode.None;
            tbPath.TextChanged += new RoutedEventHandler(tbPath_TextChanged);

        }

        #region Implementierung

        void tbPath_TextChanged(object sender, RoutedEventArgs e)
        {
            string newTxt = tbPath.Text;
            if (_curPath == null || _curPath.PathRelativeToParentFile != newTxt)
            {
                _curPath = L3dFilePath.CreateRelativeToFile(newTxt, _curPath != null ? _curPath.ParentFile : null);
                if (newTxt != "\\")
                {
                    ValidateCurrentPath();
                }
            }
        }

        void tbPath_Populating(object sender, PopulatingEventArgs e)
        {
            L3dFilePath parentFile = _curPath.ParentFile;
            string startDir;
            if (L3dFilePath.IsNullOrEmpty(parentFile) || e.Parameter.StartsWith("\\"))
            {
                startDir = L3dFilePath.LoksimDirectory.AbsolutePath;
            }
            else
            {
                startDir = parentFile.Directory;
            }

            startDir = startDir + System.IO.Path.DirectorySeparatorChar + e.Parameter;
            if (!string.IsNullOrEmpty(startDir))
            {
                string dir = System.IO.Path.GetDirectoryName(startDir);
                if (Directory.Exists(dir))
                {
                    string file = System.IO.Path.GetFileName(startDir);
                    var res = Directory.EnumerateDirectories(dir, file + "*").Select(p => L3dFilePath.CreateRelativeToFile(p, parentFile).PathRelativeToParentFile);

                    if (_possibleExtensions.Count() == 0)
                    {
                        res = res.Union(Directory.EnumerateFiles(dir, file + "*").Select(p => L3dFilePath.CreateRelativeToFile(p, parentFile).PathRelativeToParentFile));
                    }
                    else
                    {
                        foreach (var ext in _possibleExtensions)
                        {
                            res = res.Union(Directory.EnumerateFiles(dir, file + "*." + ext).Select(p => L3dFilePath.CreateRelativeToFile(p, parentFile).PathRelativeToParentFile));
                        }
                    }

                    tbPath.ItemsSource = res.Take(15);
                }
                else
                {
                    tbPath.ItemsSource = Enumerable.Empty<string>();
                }
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            string p = DialogHelpers.OpenLoksimFile(FilePath, FileDialogFilters, null);
            if (!string.IsNullOrEmpty(p))
            {
                FilePath = L3dFilePath.CreateRelativeToFile(p, FilePath != null ? FilePath.ParentFile : null);
            }
        }

        private string TbText
        {
            get { return (string)GetValue(TbTextProperty); }
            set { SetValue(TbTextProperty, value); }
        }

        // Using a DependencyProperty as the backing store for TbText.  This enables animation, styling, binding, etc...
        private static readonly DependencyProperty TbTextProperty =
            DependencyProperty.Register("TbText", typeof(string), typeof(FilePathControl), new UIPropertyMetadata(string.Empty, new PropertyChangedCallback(OnTbTextChanged)));

        private static void OnTbTextChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            string newTxt = e.NewValue as string;
            FilePathControl ctrl = (FilePathControl)d;
            if (ctrl.FilePath == null || ctrl.FilePath.PathRelativeToParentFile != newTxt)
            {
                ctrl.FilePath = L3dFilePath.CreateRelativeToFile(newTxt, ctrl.FilePath != null ? ctrl.FilePath.ParentFile : null);
            }
        }

        #endregion

        /// <summary>
        /// Dateiendungen welche bei Autocomplete verwendet werden und welche beim Datei-Öffnen Dialog angezeigt werden
        /// </summary>
        public IEnumerable<CommonFileDialogFilter> FileDialogFilters
        {
            get { return (IEnumerable<CommonFileDialogFilter>)GetValue(FileDialogFiltersProperty); }
            set { SetValue(FileDialogFiltersProperty, value); }
        }

        // Using a DependencyProperty as the backing store for FileDialogFilters.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FileDialogFiltersProperty =
            DependencyProperty.Register("FileDialogFilters", typeof(IEnumerable<CommonFileDialogFilter>), typeof(FilePathControl), new UIPropertyMetadata(null, new PropertyChangedCallback(OnFileDialogFiltersChanged)));


        private static void OnFileDialogFiltersChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            IEnumerable<CommonFileDialogFilter> newVal = e.NewValue as IEnumerable<CommonFileDialogFilter>;
            FilePathControl ctrl = (FilePathControl)d;
            if (newVal != null)
            {
                ctrl._possibleExtensions = newVal.SelectMany(el => el.Extensions).Distinct();
            }
            else
            {
                ctrl._possibleExtensions = Enumerable.Empty<string>();
            }
        }

        /// <summary>
        /// GUID für FileDialog
        /// </summary>
        public Guid FileDialogGuid
        {
            get { return (Guid)GetValue(FileDialogGuidProperty); }
            set { SetValue(FileDialogGuidProperty, value); }
        }

        // Using a DependencyProperty as the backing store for FileDialogGuid.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FileDialogGuidProperty =
            DependencyProperty.Register("FileDialogGuid", typeof(Guid), typeof(FilePathControl), new UIPropertyMetadata(null));

        /// <summary>
        /// L3dFilePath; Angezeigter Pfad
        /// <br>Für die Validierung ist es wichtig, dass hier einmal ein (eventuell leerer) Pfad gesetzt wird, welche die ParentFile enthält</br>
        /// </summary>
        public L3dFilePath FilePath
        {
            get 
            { 
                return (L3dFilePath)GetValue(FilePathProperty); 
            }
            set 
            { 
                SetValue(FilePathProperty, value);
            }
        }

        private void ValidateCurrentPath()
        {
            BindingExpression bindingExpression = BindingOperations.GetBindingExpression(tbPath, AutoCompleteBox.IsEnabledProperty);
            if (L3dFilePath.IsNullOrEmpty(_curPath))
            {
                //ctrl.TbText = string.Empty;
                tbPath.Text = string.Empty;
                Validation.ClearInvalid(bindingExpression);
            }
            else
            {
                //ctrl.TbText = newPath.PathRelativeToParentFile;
                tbPath.Text = _curPath.PathRelativeToParentFile;
                if (_curPath.Exists)
                {
                    Validation.ClearInvalid(bindingExpression);
                }
                else
                {
                    ValidationError validationError = new ValidationError(new ExceptionValidationRule(), bindingExpression);
                    validationError.ErrorContent = String.Format(Loksim3D.WetterEdit.Resources.Strings.File_0_NotFound, _curPath.PathRelativeToL3dDir);
                    Validation.MarkInvalid(bindingExpression, validationError);
                }
            }
        }

        // Using a DependencyProperty as the backing store for FilePath.  This enables animation, styling, binding, etc...
        public static readonly DependencyProperty FilePathProperty =
            DependencyProperty.Register("FilePath", typeof(L3dFilePath), typeof(FilePathControl), new UIPropertyMetadata(null, new PropertyChangedCallback(OnFilePathChanged)));
       
        private static void OnFilePathChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            L3dFilePath newPath = e.NewValue as L3dFilePath;
            FilePathControl ctrl = (FilePathControl)d;
            if (!L3dFilePath.Equals(newPath, ctrl._curPath))
            {
                ctrl._curPath = newPath;
                ctrl.ValidateCurrentPath();
            }
        }

        private void tbPath_LostFocus(object sender, RoutedEventArgs e)
        {
            FilePath = _curPath;
        }        
    }
}
