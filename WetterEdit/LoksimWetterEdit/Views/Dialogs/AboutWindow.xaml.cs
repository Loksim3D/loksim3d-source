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
using System.Reflection;
using System.Diagnostics;

namespace Loksim3D.WetterEdit.Views.Dialogs
{
    /// <summary>
    /// Interaction logic for AboutWindow.xaml
    /// </summary>
    public partial class AboutWindow : Window
    {
        public AboutWindow()
        {
            InitializeComponent();
        }


        private DateTime RetrieveLinkerTimestamp() 
        { 
            string filePath = System.Reflection.Assembly.GetCallingAssembly().Location; 
            const int c_PeHeaderOffset = 60; 
            const int c_LinkerTimestampOffset = 8; 
            byte[] b = new byte[2048]; 
            System.IO.Stream s = null; 
 
            try 
            { 
                s = new System.IO.FileStream(filePath, System.IO.FileMode.Open, System.IO.FileAccess.Read); 
                s.Read(b, 0, 2048); 
            } 
            finally 
            { 
                if (s != null) 
                { 
                    s.Close(); 
                } 
            } 
 
            int i = System.BitConverter.ToInt32(b, c_PeHeaderOffset); 
            int secondsSince1970 = System.BitConverter.ToInt32(b, i + c_LinkerTimestampOffset); 
            DateTime dt = new DateTime(1970, 1, 1, 0, 0, 0); 
            dt = dt.AddSeconds(secondsSince1970); 
            dt = dt.AddHours(TimeZone.CurrentTimeZone.GetUtcOffset(dt).Hours); 
            return dt; 
        } 

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            txtDate.Content = RetrieveLinkerTimestamp().ToShortDateString();
            txtProductVersion.Content = Assembly.GetExecutingAssembly().GetName().Version.ToString();
        }

        private void Hyperlink_Click(object sender, RoutedEventArgs e)
        {
            Process.Start(global::Loksim3D.WetterEdit.Resources.Strings.LoksimWebsite);
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }
    }
}
