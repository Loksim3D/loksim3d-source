using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Loksim3D.WetterEdit.Utils;
using Loksim3D.WetterEdit.LogicWrappers;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;
using System.ComponentModel;
using Loksim3D.WetterEdit.Utils.Attributes;

namespace Loksim3D.WetterEdit.FileWrappers
{
    [LocalizedDisplayName("DrivingCabFile")]
    public class DrivingCabFile : UndoAwareFile
    {
        public DrivingCabFile()
        {
            //TODO nur zum debuggen
            Schleudern = new OnOffGauge(this);
            Sanden = new OnOffGauge(this);
            Bitmap = L3dFilePath.CreateRelativeToL3dDir(@"\Lok\Diesel-Loks\BR 218\UPeters\BR 218 822-5.bmp");

            Schleudern.OnPosition.X = 10;
            Schleudern.OnPosition.Y = 20;
            Schleudern.OffPosition.X = 100;
            Schleudern.OffPosition.Y = 200;
            Schleudern.Dimension.Width = 1000;
            Schleudern.Dimension.Height = 2000;
        }


        protected override void LoadFromDoc(System.Xml.Linq.XDocument doc)
        {
            throw new NotImplementedException();
        }

        protected override System.Xml.Linq.XDocument SaveToXmlDocument(System.Xml.Linq.XElement rootPropsElement, L3dFilePath filePath)
        {
            throw new NotImplementedException();
        }


        
        private L3dFilePath _bitmap;

        [LocalizedCategory("Darstellung")]
        [LocalizedDisplayName("Bitmap")]
        [LocalizedDescription("Definiert das Bitmap zur Darstellung des Führerstands")]
        public L3dFilePath Bitmap
        {
            get { return _bitmap; }
            set
            {
                if (value != _bitmap)
                {
                    var oldVal = _bitmap;
                    _bitmap = value;
                    NotifyPropertyChanged(() => Bitmap, () => Bitmap = oldVal);
                }
            }
        }

        private double _engineLength;

        [LocalizedCategory("Daten Tfz")]
        [LocalizedDisplayName("Lok Länge")]
        [LocalizedDescription("Gibt die Länge der Lok an (in m)")]
        public double EngineLength
        {
            get { return _engineLength; }
            set
            {
                if (value != _engineLength)
                {
                    if (value < 0)
                    {
                        value = 0;
                    }
                    _engineLength = value;
                    NotifyPropertyChanged(() => EngineLength);
                }
            }
        }

        
        private string _author;

        [LocalizedCategory("Daten Tfz")]
        [LocalizedDisplayName("Autor")]
        [LocalizedDescription("Definiert den Autor des Führerstands")]
        public string Author
        {
            get { return _author; }
            set
            {
                if (value != _author)
                {
                    _author = value;
                    NotifyPropertyChanged(() => Author);
                }
            }
        }

        
        private OnOffGauge _schleudern;

        [LocalizedCategory("Instrumente")]
        [LocalizedDisplayName("Schleudern")]
        [LocalizedDescription("Dieses Instrument zeigt an falls das Tfz schleudert")]
        [ExpandableObject]
        public OnOffGauge Schleudern
        {
            get { return _schleudern; }
            set
            {
                if (value != _schleudern)
                {
                    var oldVal = _schleudern;
                    _schleudern = value;
                    NotifyPropertyChanged(() => Schleudern, () => Schleudern = oldVal);
                }
            }
        }

        
        private OnOffGauge _sanden;

        [LocalizedCategory("Instrumente")]
        [LocalizedDisplayName("Sanden")]
        [LocalizedDescription("Dieses Instrument zeigt an ob Sanden aktiviert ist")]
        [ExpandableObject]
        public OnOffGauge Sanden
        {
            get { return _sanden; }
            set
            {
                if (value != _sanden)
                {
                    var oldVal = _sanden;
                    _sanden = value;
                    NotifyPropertyChanged(() => Sanden, () => Sanden = oldVal);
                }
            }
        }
    }

    public class GaugePosition : UndoAwareFilePart
    {
        public GaugePosition(IUndoManager undoManager) : base(undoManager)
        {
        }

        private int _x;

        public int X
        {
            get { return _x; }
            set
            {
                if (value != _x)
                {
                    var oldVal = _x;
                    _x = value;
                    NotifyPropertyChanged(() => X, () => X = oldVal);
                }
            }
        }

        
        private int _y;

        public int Y
        {
            get { return _y; }
            set
            {
                if (value != _y)
                {
                    var oldVal = _y;
                    _y = value;
                    NotifyPropertyChanged(() => Y, () => Y = oldVal);
                }
            }
        }
    }


    public class GaugeDimension : UndoAwareFilePart
    {
        public GaugeDimension(IUndoManager undoManager)
            : base(undoManager)
        {
        }

        private int _width;

        public int Width
        {
            get { return _width; }
            set
            {
                if (value != _width)
                {
                    var oldVal = _width;
                    _width = value;
                    NotifyPropertyChanged(() => Width, () => Width = oldVal);
                }
            }
        }


        private int _height;

        public int Height
        {
            get { return _height; }
            set
            {
                if (value != _height)
                {
                    var oldVal = _height;
                    _height = value;
                    NotifyPropertyChanged(() => Height, () => Height = oldVal);
                }
            }
        }
    }

    public class OnOffGauge : UndoAwareFilePart
    {
        public OnOffGauge(IUndoManager undoManager) : base(undoManager)
        {
            _offPosition = new GaugePosition(undoManager);
            _onPosition = new GaugePosition(undoManager);
            _dimension = new GaugeDimension(undoManager);
            _position = new GaugePosition(undoManager);
        }

        public override string ToString()
        {
            return string.Empty;
        }
        

        private GaugePosition _position;

        [LocalizedDisplayName("Position Instrument")]
        [LocalizedDescription("Gibt die Position des Instruments im Führerstand an")]
        [PropertyOrder(6)]
        public GaugePosition Position 
        {
            get { return _position; }
            set
            {
                if (value != _position)
                {
                    var oldVal = _position;
                    _position = value;
                    NotifyPropertyChanged(() => Position, () => Position = oldVal);
                }
            }
        }

        private GaugePosition _onPosition;

        [LocalizedDisplayName("Ein-Position")]
        [LocalizedDescription("Gibt die Position des darzustellenden Texturausschnitts im eingeschalteten Zustand an")]
        [PropertyOrder(2)]
        public GaugePosition OnPosition
        {
            get { return _onPosition; }
            set
            {
                if (value != _onPosition)
                {
                    var oldVal = _onPosition;
                    _onPosition = value;
                    NotifyPropertyChanged(() => OnPosition, () => OnPosition = oldVal);
                }
            }
        }


        private GaugePosition _offPosition;

        [LocalizedDisplayName("Aus-Position")]
        [LocalizedDescription("Gibt die Position des darzustellenden Texturausschnitts im ausgeschalteten Zustand an")]
        [PropertyOrder(0)]
        public GaugePosition OffPosition
        {
            get { return _offPosition; }
            set
            {
                if (value != _offPosition)
                {
                    var oldVal = _offPosition;
                    _offPosition = value;
                    NotifyPropertyChanged(() => OffPosition, () => OffPosition = oldVal);
                }
            }
        }

        
        private GaugeDimension _dimension;

        [LocalizedDisplayName("Dimension Instrument")]
        [LocalizedDescription("Gibt Breite / Höhe des Instruments an")]
        [PropertyOrder(4)]
        public GaugeDimension Dimension
        {
            get { return _dimension; }
            set
            {
                if (value != _dimension)
                {
                    var oldVal = _dimension;
                    _dimension = value;
                    NotifyPropertyChanged(() => Dimension, () => Dimension = oldVal);
                }
            }
        }

        
        private bool _displayed;

        [LocalizedDisplayName("Vorhanden")]
        [LocalizedDescription("Definiert ob dieses Instrument im Führerstand vorhanden ist")]
        [PropertyOrder(8)]
        public bool Displayed
        {
            get { return _displayed; }
            set
            {
                if (value != _displayed)
                {
                    var oldVal = _displayed;
                    _displayed = value;
                    NotifyPropertyChanged(() => Displayed, () => Displayed = oldVal);
                }
            }
        }
    }
}
