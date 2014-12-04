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
using System.Windows.Shapes;
using Loksim3D.WetterEdit.FileWrappers;
using Loksim3D.WetterEdit.ViewModels;

namespace Loksim3D.WetterEdit.Views
{
    /// <summary>
    /// Interaction logic for FilePropertiesWindow.xaml
    /// </summary>
    public partial class FilePropertiesWindow : Window
    {
        private FilePropertiesViewModel viewModel;

        public FilePropertiesWindow(LoksimFile file)
        {
            InitializeComponent();
            viewModel = new FilePropertiesViewModel(file, this);
            viewModel.RequestClose += viewModel_RequestClose;
            this.DataContext = viewModel;
        }

        private void Window_GotFocus(object sender, RoutedEventArgs e)
        {
            viewModel.GotFocus(sender, e);
        }

        protected override void OnClosed(EventArgs e)
        {
            base.OnClosed(e);
            viewModel.RequestClose -= viewModel_RequestClose; 
        }

        void viewModel_RequestClose(bool obj)
        {
            this.Close();
        }
    }
}
