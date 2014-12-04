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
using Xceed.Wpf.Toolkit.PropertyGrid;
using Loksim3D.WetterEdit.FileWrappers;
using Loksim3D.WetterEdit.Utils;

namespace Loksim3D.WetterEdit.Views
{
    /// <summary>
    /// Interaction logic for DrivingCabFileEdit.xaml
    /// </summary>
    public partial class DrivingCabFileEdit : UserControl
    {
        private const int MIN_MOUSE_MOVE_DRAG = 2;
        private Point lastDown;
        private double scale = 1;

        public DrivingCabFileEdit()
        {
            InitializeComponent();
        }

        private void PropertyGrid_SelectedPropertyItemChanged(object sender, RoutedEventArgs e)
        {
            RefreshDisplayedRects();
        }

        private void RefreshDisplayedRects()
        {
            canvasBmp.Children.RemoveRange(1, canvasBmp.Children.Count - 1);

            string propName;
            OnOffGauge g = GetSelectedGauge(out propName);

            if (g != null)
            {
                AddRect(g.Position.X, g.Position.Y, g.Dimension.Width, g.Dimension.Height, Brushes.Gray);

                if (g.Position.X != g.OffPosition.X || g.Position.Y != g.OffPosition.Y)
                {
                    AddRect(g.OffPosition.X, g.OffPosition.Y, g.Dimension.Width, g.Dimension.Height, Brushes.Red);
                }

                if (g.Position.X != g.OnPosition.X || g.Position.Y != g.OnPosition.Y)
                {
                    AddRect(g.OnPosition.X, g.OnPosition.Y, g.Dimension.Width, g.Dimension.Height, Brushes.Green);
                }
            }
        }

        private OnOffGauge GetSelectedGauge(out string selectedPropName)
        {
            /*
             * The Xceed version of the extendes toolkit changed some parts of the interface
             * of the original extended wpf toolkit
            PropertyItem selProp = propGrid.SelectedProperty;

            //selProp.PropertyGrid.SelectedProperty.
            OnOffGauge g = null;
            DrivingCabFile f = propGrid.SelectedObject as DrivingCabFile;
            selectedPropName = null;
            if (selProp != null && selProp.Level == 1 && f != null)
            {
                string propName = selProp.BindingPath.Substring(0, selProp.BindingPath.IndexOf('.'));
                g = f.GetType().InvokeMember(propName, System.Reflection.BindingFlags.GetProperty, null, f, null) as OnOffGauge;
                selectedPropName = selProp.BindingPath.Substring(selProp.BindingPath.IndexOf('.') + 1);
            }
            else if (selProp != null && f != null)
            {
                g = selProp.Value as OnOffGauge;
            }
            return g;
             */
            selectedPropName = null;
            return null;
        }

        private void AddRect(int x, int y, int width, int height, Brush brush)
        {
            Rectangle r;
            for (int i = 0; i < 2; i++)
            {
                r = new Rectangle();
                r.Fill = null;
                if (i == 0)
                {
                    r.Stroke = Brushes.White;
                }
                else
                {
                    r.Stroke = brush;
                    r.StrokeDashArray = new DoubleCollection { 8.0, 8.0 };
                }
                r.StrokeThickness = 1;
                Canvas.SetLeft(r, x * scale);
                Canvas.SetTop(r, y * scale);
                r.Width = width * scale;
                r.Height = height * scale;
                this.canvasBmp.Children.Add(r);
            }
        }

        private void imgCab_SourceUpdated(object sender, DataTransferEventArgs e)
        {
            imgCab_SizeChanged(sender, null);
        }

        private void imgCab_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            Image img = sender as Image;
            if (img != null)
            {
                canvasBmp.Height = img.Source.Height * scale;
                canvasBmp.Width = img.Source.Width * scale;
            }
        }

        private void canvasBmp_MouseDown(object sender, MouseButtonEventArgs e)
        {          
            lastDown = e.GetPosition(imgCab);
            string selPropName;
            OnOffGauge g = GetSelectedGauge(out selPropName);
            if (g != null)
            {
                if (selPropName == null || selPropName == PropertySupport.ExtractPropertyName(() => g.Position))
                {
                    g.Position.X = (int)(lastDown.X);
                    g.Position.Y = (int)(lastDown.Y);
                }
                else if (selPropName == PropertySupport.ExtractPropertyName(() => g.OnPosition))
                {
                    g.OnPosition.X = (int)(lastDown.X);
                    g.OnPosition.Y = (int)(lastDown.Y);
                }
                else if (selPropName == PropertySupport.ExtractPropertyName(() => g.OffPosition))
                {
                    g.OffPosition.X = (int)(lastDown.X);
                    g.OffPosition.Y = (int)(lastDown.Y);
                }
                RefreshDisplayedRects();
            }
        }

        private void canvasBmp_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                Point curPos = e.GetPosition(imgCab);
                string selPropName;
                OnOffGauge g = GetSelectedGauge(out selPropName);
                if (g != null && Math.Abs(curPos.X - lastDown.X) > MIN_MOUSE_MOVE_DRAG / scale && Math.Abs(curPos.Y - lastDown.Y) > MIN_MOUSE_MOVE_DRAG / scale)
                {
                    int newX = (int)(Math.Min(curPos.X, lastDown.X));
                    int newY = (int)(Math.Min(curPos.Y, lastDown.Y));
                    int newWidth = (int)(Math.Abs(curPos.X - lastDown.X));
                    int newHeight = (int)(Math.Abs(curPos.Y - lastDown.Y));
                    if (selPropName == PropertySupport.ExtractPropertyName(() => g.OnPosition))
                    {
                        g.OnPosition.X = newX;
                        g.OnPosition.Y = newY;
                    }
                    else if (selPropName == PropertySupport.ExtractPropertyName(() => g.OffPosition))
                    {
                        g.OffPosition.X = newX;
                        g.OffPosition.Y = newY;
                    }
                    else
                    {
                        g.Position.X = newX;
                        g.Position.Y = newY;
                    }
                    g.Dimension.Width = newWidth;
                    g.Dimension.Height = newHeight;
                    RefreshDisplayedRects();
                }
            }
        }

        private void canvasBmp_MouseUp(object sender, MouseButtonEventArgs e)
        {
        }

        private void canvasBmp_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl))
            {
                double mult = e.Delta > 0 ? 2 : 0.5;
                imgScale.ScaleX *= mult;
                imgScale.ScaleY *= mult;
                scale = imgScale.ScaleX;
                imgCab_SizeChanged(imgCab, null);
                RefreshDisplayedRects();
            }
        }
    }
}
