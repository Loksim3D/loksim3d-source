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

namespace Loksim3D.WetterEdit.Views.Dialogs
{
    /// <summary>
    /// Interaction logic for NewFileWindow.xaml
    /// </summary>
    public partial class NewFileWindow : Window
    {
        public NewFileWindow()
        {
            InitializeComponent();
        }

        private void ListView_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            ButtonOK_Click(null, null);
        }

        private void ButtonOK_Click(object sender, RoutedEventArgs e)
        {
            ListViewItem it = listView.SelectedItem as ListViewItem;
            if (it != null && it.Tag != null)
            {
                switch (it.Tag.ToString())
                {
                    case "l3dsky":
                        SelectedFile = new SkyFile();
                        break;
                    case "l3dwth":
                        SelectedFile = WeatherFile.CreateNewDefaultFile();
                        break;
                    case "l3dfst":
                        SelectedFile = new DrivingCabFile();
                        break;
                }
                DialogResult = true;
                Close();
            }
        }

        private void ButtonCancel_Click(object sender, RoutedEventArgs e)
        {
            DialogResult = false;
            Close();
        }

        public LoksimFile SelectedFile
        {
            get;
            private set;
        }
    }
}
