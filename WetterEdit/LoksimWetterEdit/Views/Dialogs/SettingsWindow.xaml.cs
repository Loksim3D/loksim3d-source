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
using Loksim3D.WetterEdit.Settings;
using Loksim3D.WetterEdit.Resources;

namespace Loksim3D.WetterEdit.Views
{
    /// <summary>
    /// Interaction logic for SettingsWindow.xaml
    /// </summary>
    public partial class SettingsWindow : Window
    {
        public SettingsWindow()
        {
            List<ComboBoxItem> startModes = new List<ComboBoxItem>();
            startModes.Add(new ComboBoxItem { Content = Strings.SettingsUacStartModeAsk, Tag = RegistrySettings.AdminStartMode.Ask });
            startModes.Add(new ComboBoxItem { Content = Strings.UacDlgBtnAdmin, Tag = RegistrySettings.AdminStartMode.StartAsAdmin });
            startModes.Add(new ComboBoxItem { Content = Strings.UacDlgBtnNoAdmin, Tag = RegistrySettings.AdminStartMode.DoNotStartAsAdmin });
            UacStartModeItems = startModes;

            this.DataContext = this;
            InitializeComponent();
        }

        public RegistrySettings Settings
        {
            get 
            {
                return Loksim3D.WetterEdit.Settings.RegistrySettings.Default;
            }
        }

        public ICollection<ComboBoxItem> UacStartModeItems
        {
            get;
            set;
        }

        public bool UacVisible
        {
            get
            {
                return Environment.OSVersion.Version.Major >= 6;
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            comboLang.SelectedItem = comboLang.Items.Cast<ComboBoxItem>().FirstOrDefault(it =>
                String.Compare(RegistrySettings.Default.UiCulture, it.Tag + "", StringComparison.InvariantCultureIgnoreCase) == 0);

            cbUac.SelectedItem = UacStartModeItems.FirstOrDefault(it => (it.Tag as RegistrySettings.AdminStartMode?) == RegistrySettings.Default.DefaultAdminStartMode);
        }

        private void OkButton_Click(object sender, RoutedEventArgs e)
        {
            if (comboLang.SelectedItem != null)
            {
                RegistrySettings.Default.UiCulture = ((ComboBoxItem)comboLang.SelectedItem).Tag + "";
            }
            if (cbUac.SelectedItem != null)
            {
                RegistrySettings.Default.DefaultAdminStartMode = (RegistrySettings.AdminStartMode)((ComboBoxItem)cbUac.SelectedItem).Tag;
            }
            Close();
        }
    }
}
