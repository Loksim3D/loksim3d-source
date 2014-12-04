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
using System.ComponentModel;
using System.Diagnostics;
using Loksim3D.WetterEdit.ViewModels;

namespace Loksim3D.WetterEdit.Views
{
    /// <summary>
    /// Interaction logic for WeatherFileEdit.xaml
    /// </summary>
    public partial class WeatherFileEdit : UserControl
    {
        private System.Reflection.MethodInfo texOneProp;
        private System.Reflection.MethodInfo texTwoProp;
        private System.Reflection.MethodInfo weatherCtrlProp;

        public WeatherFileEdit()
        {
            InitializeComponent();
        }

        private void parentUserCtrl_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            texOneProp = null;
            texTwoProp = null;
            if (e.OldValue != null)
            {
                if (weatherCtrlProp != null)
                {
                    object weatherCtrl = weatherCtrlProp.Invoke(e.OldValue, null);
                    if (weatherCtrl is INotifyPropertyChanged)
                    {
                        ((INotifyPropertyChanged)weatherCtrl).PropertyChanged -= TestTextureChanged;
                    }
                }
                weatherCtrlProp = null;
            }
            if (e.NewValue != null)
            {
                object weatherCtrl = null;
                System.Reflection.PropertyInfo weatherTestProp = e.NewValue.GetType().GetProperty("WeatherTest");
                if (weatherTestProp != null)
                {
                    weatherCtrlProp = weatherTestProp.GetGetMethod();
                    weatherCtrl = weatherCtrlProp.Invoke(e.NewValue, null);
                    if (weatherCtrl is INotifyPropertyChanged)
                    {
                        System.Reflection.PropertyInfo p = null;
                        p = weatherCtrl.GetType().GetProperty("TextureOne");
                        if (p != null)
                        {
                            texOneProp = p.GetGetMethod();
                        }
                        p = weatherCtrl.GetType().GetProperty("TextureTwo");
                        if (p != null)
                        {
                            texTwoProp = p.GetGetMethod();
                        }
                        ((INotifyPropertyChanged)weatherCtrl).PropertyChanged += TestTextureChanged;
                    }
                }

                TestTextureChanged(weatherCtrl, new PropertyChangedEventArgs("TextureOne"));
                TestTextureChanged(weatherCtrl, new PropertyChangedEventArgs("TextureTwo"));
            }

            if (texOneProp == null)
            {
                testImgCtrl.Source = null;
                Debug.WriteLine("Cannot bind to WeatherTest.TextureOne property");
            }
            if (texTwoProp == null)
            {
                testImgEffect.Input2 = null;
                Debug.WriteLine("Cannot bind to WeatherTest.TextureTwo property");
            }
        }

        private void TestTextureChanged(object sender, PropertyChangedEventArgs e)
        {
            try
            {
                if (e.PropertyName == "TextureOne" && texOneProp != null)
                {
                    object val = null;
                    if (sender != null && (val = texOneProp.Invoke(sender, null)) != null && val is L3dFilePath)
                    {
                        BitmapImage bmp = new BitmapImage(new Uri(((L3dFilePath)val).AbsolutePath));
                        testImgCtrl.Source = bmp;
                    }
                    else
                    {
                        testImgCtrl.Source = null;
                    }
                }
                else if (e.PropertyName == "TextureTwo" && texTwoProp != null)
                {
                    object val = null;
                    if (sender != null && (val = texTwoProp.Invoke(sender, null)) != null && val is L3dFilePath)
                    {
                        BitmapImage bmp = new BitmapImage(new Uri(((L3dFilePath)val).AbsolutePath));
                        testImgEffect.Input2 = new ImageBrush(bmp);
                    }
                    else
                    {
                        testImgEffect.Input2 = null;
                    }
                }
            }
            catch (Exception)
            {
                testImgCtrl.Source = null;
                testImgEffect.Input2 = null;
            }
        }


        static T FindVisualParent<T>(UIElement element) where T : UIElement
        {
            UIElement parent = element;
            while (parent != null)
            {
                T correctlyTyped = parent as T;
                if (correctlyTyped != null)
                {
                    return correctlyTyped;
                }

                parent = VisualTreeHelper.GetParent(parent) as UIElement;
            }
            return null;
        }

    }
}
