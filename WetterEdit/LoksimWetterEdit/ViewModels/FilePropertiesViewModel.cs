using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.FileWrappers;
using System.Windows;
using System.Windows.Input;
using Loksim3D.WetterEdit.LogicWrappers;
using System.Windows.Media.Imaging;
using System.Drawing;
using System.Windows.Interop;
using System.Windows.Media;
using System.Diagnostics;
using System.Windows.Threading;
using Loksim3D.WetterEdit.Views;
using Loksim3D.WetterEdit.ViewModels.Helpers;


namespace Loksim3D.WetterEdit.ViewModels
{
    /// <summary>
    /// ViewModel für den Datei-Eigenschaften Dialog
    /// </summary>
    class FilePropertiesViewModel : BaseModel
    {
        private string _fileAuthor;
        private string _fileInfo;
        private BitmapSource _filePicture;
        private bool _picChanged;
        private L3dFilePath _fileDoku;

        public event Action<bool> RequestClose;

        private Window _parentWindow;

        private void SetFilePicSource(L3dFilePath file)
        {
            try
            {
                BitmapImage b = new BitmapImage();
                // BitmapImage.UriSource must be in a BeginInit/EndInit block
                b.BeginInit();
                b.UriSource = new Uri(file.AbsolutePath);
                //b.CacheOption = BitmapCacheOption.OnLoad;
                // To save significant application memory, set the DecodePixelWidth or  
                // DecodePixelHeight of the BitmapImage value of the image source to the desired 
                // height or width of the rendered image. If you don't do this, the application will 
                // cache the image as though it were rendered as its normal size rather then just 
                // the size that is displayed.
                // Note: In order to preserve aspect ratio, set DecodePixelWidth
                // or DecodePixelHeight but not both.
                b.DecodePixelWidth = 512;                
                b.CacheOption = BitmapCacheOption.OnLoad;
                b.EndInit();
                FilePicture = b;
            }
            catch (Exception)
            {
            }
        }

        public FilePropertiesViewModel(LoksimFile file, Window parentWindow)
        {
            _fileAuthor = file.FileAuthor;
            _fileInfo = file.FileInfo;
            _fileDoku = file.FileDoku;

            _parentWindow = parentWindow;
            if (!L3dFilePath.IsNullOrEmpty(file.FilePicture))
            {
                SetFilePicSource(file.FilePicture);
            }

            OtherPictureCmd = new RelayCommand(obj =>
                {
                    string s = DialogHelpers.OpenLoksimFile(file.FilePicture.AbsolutePath, FileExtensions.ImageFiles, _parentWindow);
                    if (!string.IsNullOrEmpty(s))
                    {
                        L3dFilePath p = new L3dFilePath(s);
                        RemovePictureCmd.Execute(null);
                        Dispatcher.CurrentDispatcher.BeginInvoke(new Action(() =>
                            {
                                SetFilePicSource(p);
                            }), 
                            System.Windows.Threading.DispatcherPriority.Background);
                    }
                },
                obj =>
                {
                    return !L3dFilePath.IsNullOrEmpty(file.OwnPath);
                });

            PictureFromClipboardCmd = new RelayCommand(obj =>
                {
                    if (Clipboard.ContainsImage())
                    {
                        FilePicture = Clipboard.GetImage();
                    }
                },
                obj =>
                {
                    return Clipboard.ContainsImage() && !L3dFilePath.IsNullOrEmpty(file.OwnPath);
                });

            RemovePictureCmd = new RelayCommand(obj =>
                {
                    FilePicture = null;
                },
                obj =>
                {
                    return FilePicture != null;
                });

            OkCmd = new RelayCommand(obj =>
                {
                    file.FileAuthor = FileAuthor;
                    file.FileInfo = FileInfo;
                    file.FileDoku = FileDoku;

                    try
                    {
                        if (_picChanged)
                        {
                            if (_filePicture != null)
                            {
                                Image bmp = null;
                                JpegBitmapEncoder enc = new JpegBitmapEncoder();
                                enc.Frames.Add(BitmapFrame.Create(_filePicture));
                                using (System.IO.MemoryStream memStream = new System.IO.MemoryStream())
                                {
                                    enc.Save(memStream);
                                    bmp = new Bitmap(memStream);
                                }
                                using (Image img = ImageHelper.ScaleBitmap(bmp, 512, 512))
                                {
                                    file.FilePicture = new L3dFilePath(file.OwnPath.AbsolutePath + ".jpg");
                                    img.Save(file.FilePicture.AbsolutePath, System.Drawing.Imaging.ImageFormat.Jpeg);
                                }                                
                                bmp.Dispose();
                            }
                            else
                            {
                                file.FilePicture = null;
                            }
                        }
                        FilePicture = null;
                    } catch (Exception ex)
                    {
                        Debug.WriteLine(ex);
                    }

                    if (RequestClose != null)
                    {
                        RequestClose(true);
                    }
                },
                obj =>
                {
                    return true;
                });

            CancelCmd = new RelayCommand(obj =>
                {
                    if (RequestClose != null)
                    {
                        RequestClose(false);
                    }
                },
                obj =>
                {
                    return true;
                });

            _picChanged = false;
        }

        public string FileAuthor 
        {
            get { return _fileAuthor; }
            set
            {
                if (value != _fileAuthor)
                {
                    _fileAuthor = value;
                    NotifyPropertyChanged("FileAuthor");
                }
            }
        }

        public string FileInfo
        {
            get { return _fileInfo; }
            set
            {
                if (value != _fileInfo)
                {
                    _fileInfo = value;
                    NotifyPropertyChanged("FileInfo");
                }
            }
        }

        public L3dFilePath FileDoku
        {
            get { return _fileDoku; }
            set    
            {
                if (value != _fileDoku)
                {
                    _fileDoku = value;
                    NotifyPropertyChanged("FileDoku");
                }
            }
        }

        public BitmapSource FilePicture
        {
            get { return _filePicture; }
            set
            {
                if (value != _filePicture)
                {
                    _filePicture = value;
                    _picChanged = true;
                    NotifyPropertyChanged("FilePicture");
                }
            }
        }

        public RelayCommand OtherPictureCmd { get; private set; }
        public RelayCommand PictureFromClipboardCmd { get; private set; }
        public RelayCommand RemovePictureCmd { get; private set; }

        public RelayCommand OkCmd { get; private set; }
        public RelayCommand CancelCmd { get; private set; }

        public void GotFocus(object sender, RoutedEventArgs e)
        {
            OtherPictureCmd.FireCanExecuteChanged(this, new EventArgs());
        }
    }
}
